(* Copyright 2023 Functori <contact@functori.com>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. *)

open Tezos_protocol_022_PsRiotum
open Test_c_parser_utils

let pp_opt_field pp ppf = function
  | None -> Format.fprintf ppf "Field unset"
  | Some v -> Format.fprintf ppf "%a" pp v

let pp_tz ppf tz = Format.fprintf ppf "%a XTZ" Protocol.Alpha_context.Tez.pp tz

let pp_lazy_expr ppf lazy_expr =
  let expr = Result.get_ok @@ Protocol.Script_repr.force_decode lazy_expr in
  Format.fprintf ppf "%s" @@ Test_micheline_c_parser.to_string expr

let pp_string_binary ppf s = Format.fprintf ppf "%a" Hex.pp (Hex.of_string s)

let pp_serialized_proof ppf proof =
  let proof =
    Data_encoding.(
      proof
      |> Binary.to_bytes_exn
           Protocol.Alpha_context.Sc_rollup.Proof.serialized_encoding
      |> Binary.of_bytes_exn (string' Hex))
  in
  pp_string_binary ppf proof

let to_string
    ( (_shell : Tezos_base.Operation.shell_header),
      (Contents_list contents : Protocol.Alpha_context.packed_contents_list) ) =
  let open Protocol.Alpha_context in
  let manager_to_string (type t)
      (Manager_operation { source; fee; operation; storage_limit; _ } :
        t Kind.manager contents) =
    let aux ~kind operation_fields =
      let manager_fields =
        [
          kind;
          Format.asprintf "%a" Environment.Signature.Public_key_hash.pp source;
          Format.asprintf "%a" pp_tz fee;
          Z.to_string storage_limit;
        ]
      in
      String.concat "" (manager_fields @ operation_fields)
    in
    match operation with
    | Delegation public_key_hash_opt ->
        aux ~kind:"Delegation"
          [
            Format.asprintf "%a"
              (pp_opt_field Environment.Signature.Public_key_hash.pp)
              public_key_hash_opt;
          ]
    | Increase_paid_storage { amount_in_bytes; destination } ->
        aux ~kind:"Increase paid storage"
          [
            Z.to_string amount_in_bytes;
            Format.asprintf "%a" Protocol.Contract_hash.pp destination;
          ]
    | Origination { delegate; script = { code; storage }; credit } ->
        aux ~kind:"Origination"
          [
            Format.asprintf "%a" pp_tz credit;
            Format.asprintf "%a"
              (pp_opt_field Environment.Signature.Public_key_hash.pp)
              delegate;
            Format.asprintf "%a" pp_lazy_expr code;
            Format.asprintf "%a" pp_lazy_expr storage;
          ]
    | Register_global_constant { value } ->
        aux ~kind:"Register global constant"
          [ Format.asprintf "%a" pp_lazy_expr value ]
    | Reveal public_key ->
        aux ~kind:"Reveal"
          [
            Format.asprintf "%a" Environment.Signature.Public_key.pp public_key;
          ]
    | Set_deposits_limit tez_opt ->
        aux ~kind:"Set deposit limit"
          [ Format.asprintf "%a" (pp_opt_field pp_tz) tez_opt ]
    | Transaction { amount; entrypoint; destination; parameters } ->
        let parameters =
          if
            Protocol.Script_repr.is_unit_parameter parameters
            && Entrypoint.is_default entrypoint
          then []
          else
            [
              Format.asprintf "%a" Entrypoint.pp entrypoint;
              Format.asprintf "%a" pp_lazy_expr parameters;
            ]
        in
        aux ~kind:"Transaction"
          ([
             Format.asprintf "%a" pp_tz amount;
             Format.asprintf "%a" Contract.pp destination;
           ]
          @ parameters)
    | Transfer_ticket
        { contents; ty; ticketer; amount; destination; entrypoint } ->
        aux ~kind:"Transfer ticket"
          [
            Format.asprintf "%a" pp_lazy_expr contents;
            Format.asprintf "%a" pp_lazy_expr ty;
            Format.asprintf "%a" Contract.pp ticketer;
            Format.asprintf "%s"
              Protocol.Script_int.(to_string (amount :> n num));
            Format.asprintf "%a" Contract.pp destination;
            Format.asprintf "%a" Entrypoint.pp entrypoint;
          ]
    | Update_consensus_key { public_key; proof } ->
        let proof =
          match proof with
          | None -> []
          | Some proof ->
              [ Format.asprintf "%a" Environment.Signature.pp proof ]
        in
        aux ~kind:"Set consensus key"
          ([
             Format.asprintf "%a" Environment.Signature.Public_key.pp public_key;
           ]
          @ proof)
    | Sc_rollup_add_messages { messages } ->
        let message_to_string message =
          Format.asprintf "%a" pp_string_binary message
        in
        aux ~kind:"SR: send messages" @@ List.map message_to_string messages
    | Sc_rollup_execute_outbox_message
        { rollup; cemented_commitment; output_proof } ->
        aux ~kind:"SR: execute outbox message"
          [
            Format.asprintf "%a" Sc_rollup.Address.pp rollup;
            Format.asprintf "%a" Sc_rollup.Commitment.Hash.pp
              cemented_commitment;
            Format.asprintf "%a" pp_string_binary output_proof;
          ]
    | Sc_rollup_originate { kind; boot_sector; parameters_ty; whitelist } ->
        let whitelist =
          match whitelist with
          | None -> []
          | Some whitelist ->
              List.map
                (Format.asprintf "%a" Environment.Signature.Public_key_hash.pp)
                whitelist
        in
        aux ~kind:"SR: originate"
          ([
             Format.asprintf "%a" Sc_rollup.Kind.pp kind;
             Format.asprintf "%a" pp_string_binary boot_sector;
             Format.asprintf "%a" pp_lazy_expr parameters_ty;
           ]
          @ whitelist)
    | _ -> assert false
  in
  let operation_to_string (type t) (operation : t contents) =
    let aux ~kind operation_fields =
      String.concat "" (kind :: operation_fields)
    in
    match operation with
    | Failing_noop message ->
        aux ~kind:"Failing noop"
          [ Format.asprintf "%a" pp_string_binary message ]
    | Proposals { source; period; proposals } ->
        aux ~kind:"Proposals"
          ([
             Format.asprintf "%a" Environment.Signature.Public_key_hash.pp
               source;
             Format.asprintf "%ld" period;
           ]
          @ List.map
              (Format.asprintf "%a" Tezos_crypto.Hashed.Protocol_hash.pp)
              proposals)
    | Ballot { source; period; proposal; ballot } ->
        aux ~kind:"Ballot"
          [
            Format.asprintf "%a" Environment.Signature.Public_key_hash.pp source;
            Format.asprintf "%ld" period;
            Format.asprintf "%a" Tezos_crypto.Hashed.Protocol_hash.pp proposal;
            Format.asprintf "%a" Vote.pp_ballot ballot;
          ]
    | Manager_operation _ | _ -> assert false
  in
  let rec operations_to_string : type t. t contents_list -> string = function
    | Single (Manager_operation _ as m) -> manager_to_string m
    | Cons ((Manager_operation _ as m), rest) ->
        manager_to_string m ^ operations_to_string rest
    | Single op -> operation_to_string op
  in
  operations_to_string contents

let to_bytes op =
  Data_encoding.Binary.to_bytes_exn
    Protocol.Alpha_context.Operation.unsigned_encoding op

external cparse_step :
  cparse_state ->
  input:bytes * int * int ->
  output:bytes * int * int ->
  int * int * st = "operation_cparse_step"
