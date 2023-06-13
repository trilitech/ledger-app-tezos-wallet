(* Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 Functori <contact@functori.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. *)

open Tezos_protocol_016_PtMumbai
open Tezos_benchmarks_proto_016_PtMumbai

(* Only use this function to generate values *)
let gen_expr =
  let gen rand =
    Protocol.Script_repr.lazy_expr
      (Michelson_generation.make_code_sampler rand Gen_micheline.config).term
  in
  let shrink _ = assert false in
  QCheck2.Gen.make_primitive ~gen ~shrink

let gen_algo =
  QCheck2.Gen.oneofl Tezos_crypto.Signature.[ Ed25519; Secp256k1; P256 ]

let gen_keys =
  let open QCheck2.Gen in
  let* seed = QCheck2.Gen.bytes_size (return 32) in
  let* algo = gen_algo in
  return @@ Tezos_crypto.Signature.generate_key ~algo ~seed ()

let gen_secret_key = QCheck2.Gen.map (fun (_, _, sk) -> sk) gen_keys
let gen_public_key = QCheck2.Gen.map (fun (_, pk, _) -> pk) gen_keys
let gen_public_key_hash = QCheck2.Gen.map (fun (pkh, _, _) -> pkh) gen_keys

let gen_tez =
  QCheck2.Gen.map Protocol.Alpha_context.Tez.(mul_exn one_cent)
  @@ QCheck2.Gen.int_range 1 100

let gen_manager_counter =
  QCheck2.Gen.map
    Protocol.Alpha_context.Manager_counter.Internal_for_tests.of_int
  @@ QCheck2.Gen.int_range 0 1000

let gen_z_bound b = QCheck2.Gen.map Z.of_int QCheck2.Gen.(int_bound b)

let gen_gaz_bound b =
  QCheck2.Gen.map Protocol.Alpha_context.Gas.Arith.integral_of_int_exn
    QCheck2.Gen.(int_bound b)

let gen_origination_nonce =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let rec incr i nonce =
    if i <= 0 then return nonce
    else
      let nonce = Origination_nonce.Internal_for_tests.incr nonce in
      incr (i - 1) nonce
  in
  let* i = int_range 1 100 in
  let initial_nonce =
    Origination_nonce.Internal_for_tests.initial Environment.Operation_hash.zero
  in
  incr i initial_nonce

let gen_contract =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let gen_originated =
    let* public_key_hash = gen_public_key_hash in
    return (Contract.Implicit public_key_hash)
  in
  let gen_implict =
    let* origination_nonce = gen_origination_nonce in
    return (Contract.Internal_for_tests.originated_contract origination_nonce)
  in
  oneof [ gen_implict; gen_originated ]

let gen_entrypoint =
  QCheck2.Gen.oneofl
    Protocol.Alpha_context.Entrypoint.
      [
        default;
        root;
        do_;
        set_delegate;
        remove_delegate;
        deposit;
        of_string_strict_exn "jean_bob";
      ]

let gen_transaction =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let* amount = gen_tez in
  let* destination = gen_contract in
  let* entrypoint = gen_entrypoint in
  let* parameters = gen_expr in
  return (Transaction { amount; destination; entrypoint; parameters })

let gen_manager_operation gen_operation =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let* source = gen_public_key_hash in
  let* fee = gen_tez in
  let* counter = gen_manager_counter in
  let* gas_limit = gen_gaz_bound 1000 in
  let* storage_limit = gen_z_bound 1000 in
  let* operation = gen_operation in
  return
    (Manager_operation
       { source; fee; counter; operation; gas_limit; storage_limit })

type hidden_manager_operation_list =
  | H :
      _ Protocol.Alpha_context.Kind.manager Protocol.Alpha_context.contents_list
      -> hidden_manager_operation_list

let gen_packed_contents_list =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let* size = int_range 1 3 in
  let rec aux i =
    if i <= 1 then
      let* transaction = gen_manager_operation gen_transaction in
      return (H (Single transaction))
    else
      let* transaction = gen_manager_operation gen_transaction in
      let* (H contents_list) = aux (i - 1) in
      return (H (Cons (transaction, contents_list)))
  in
  let* (H contents_list) = aux size in
  return (Contents_list contents_list)

let gen () =
  let shell =
    { Tezos_base.Operation.branch = Tezos_crypto.Hashed.Block_hash.zero }
  in
  let contents =
    QCheck2.Gen.generate1 ~rand:Gen_utils.random_state gen_packed_contents_list
  in
  (shell, contents)

let op = Seq.forever gen
let operation_watermark = Tezos_crypto.Signature.Generic_operation

let operation_watermark_bytes =
  Tezos_crypto.Signature.bytes_of_watermark operation_watermark

let encode op =
  Bytes.cat operation_watermark_bytes
    (Data_encoding.Binary.to_bytes_exn
       Protocol.Alpha_context.Operation.unsigned_encoding op)

let decode bin =
  let watermark = Bytes.sub bin 0 1 in
  if not @@ Bytes.equal watermark operation_watermark_bytes then
    failwith @@ Format.sprintf "%s: invalid watermark" __FUNCTION__;
  let bin = Bytes.sub bin 1 (Bytes.length bin - 1) in
  Data_encoding.Binary.of_bytes_exn
    Protocol.Alpha_context.Operation.unsigned_encoding bin

let hex =
  Seq.map
    (fun op ->
      let bin = encode op in
      let hex = Hex.of_bytes bin in
      (op, bin, hex))
    op
