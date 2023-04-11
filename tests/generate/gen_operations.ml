(* Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. *)

open Tezos_protocol_015_PtLimaPt
open Tezos_benchmarks_proto_015_PtLimaPt

let gen () =
  let open Protocol.Alpha_context in
  let source = Tezos_crypto.Signature.Public_key_hash.zero in
  let fee = Tez.fifty_cents in
  let counter = Z.of_int (Random.int 1000) in
  let gas_limit = Gas.Arith.integral_of_int_exn (Random.int 1000) in
  let storage_limit = Z.of_int (Random.int 1000) in
  let operation =
    let amount = Tez.fifty_cents in
    let destination = Contract.Originated Protocol.Contract_hash.zero in
    let entrypoint = Entrypoint.of_string_strict_exn "jean_bob" in
    let parameters =
      Protocol.Script_repr.lazy_expr
        (Michelson_generation.make_code_sampler Gen_micheline.st
           Gen_micheline.config)
          .term
    in
    Transaction { amount; destination; entrypoint; parameters }
  in
  let shell = { Tezos_base.Operation.branch = Tezos_crypto.Block_hash.zero } in
  let contents =
    Contents_list
      (Single
         (Manager_operation
            { source; fee; counter; operation; gas_limit; storage_limit }))
  in
  (shell, contents)

let op = Seq.forever gen

let bin =
  Seq.map
    (fun op ->
      ( op,
        Data_encoding.Binary.to_bytes_exn
          Protocol.Alpha_context.Operation.unsigned_encoding op ))
    op

let hex =
  Seq.map
    (fun (op, bin) ->
      let bin =
        Bytes.cat
          Tezos_crypto.Signature.(bytes_of_watermark Generic_operation)
          bin
      in
      (op, bin, `Hex (Hex.show (Hex.of_bytes bin))))
    bin
