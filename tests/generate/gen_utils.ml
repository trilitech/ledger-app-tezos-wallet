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

open Tezos_protocol_022_PsRiotum
open Tezos_micheline

let random_state =
  match Sys.getenv_opt "RAND" with
  | Some "1" -> Random.State.make_self_init ()
  | _ -> Random.get_state ()

let micheline_too_large_or_too_deep expr =
  try
    let rec traverse d expr =
      if d > 40 then raise Exit
      else
        match expr with
        | Micheline.Int (_, n) -> if Z.size n * 64 > 256 then raise Exit
        | Micheline.String _ | Micheline.Bytes _ -> ()
        | Micheline.Prim (_, _, n, _) | Micheline.Seq (_, n) ->
            List.iter (traverse (d + 1)) n
    in
    traverse 0 (Micheline.root expr);
    false
  with Exit -> true

let lazy_expr_too_large_or_too_deep lazy_expr =
  let expr = Result.get_ok @@ Protocol.Script_repr.force_decode lazy_expr in
  micheline_too_large_or_too_deep expr

let operations_too_large_or_too_deep
    ( (_shell : Tezos_base.Operation.shell_header),
      (Contents_list contents : Protocol.Alpha_context.packed_contents_list) ) =
  let open Protocol.Alpha_context in
  let traverse_manager (type t)
      (Manager_operation { operation; _ } : t Kind.manager contents) =
    match operation with
    | Origination { script = { code; storage }; _ } ->
        lazy_expr_too_large_or_too_deep code
        || lazy_expr_too_large_or_too_deep storage
    | Register_global_constant { value } ->
        lazy_expr_too_large_or_too_deep value
    | Transaction { parameters; _ } ->
        lazy_expr_too_large_or_too_deep parameters
    | Transfer_ticket { contents; ty; _ } ->
        lazy_expr_too_large_or_too_deep contents
        || lazy_expr_too_large_or_too_deep ty
    | Sc_rollup_originate { parameters_ty; _ } ->
        lazy_expr_too_large_or_too_deep parameters_ty
    | Delegation _ | Increase_paid_storage _ | Reveal _ | Set_deposits_limit _
    | Update_consensus_key _ | Sc_rollup_add_messages _
    | Sc_rollup_execute_outbox_message _ ->
        false
    | _ -> assert false
  in
  let traverse_operation (type t) (operation : t contents) =
    match operation with
    | Proposals _ | Ballot _ | Failing_noop _ -> false
    | Manager_operation _ | _ -> assert false
  in
  let rec traverse_contents : type t. t contents_list -> bool = function
    | Single (Manager_operation _ as m) -> traverse_manager m
    | Cons ((Manager_operation _ as m), rest) ->
        traverse_manager m || traverse_contents rest
    | Single op -> traverse_operation op
  in
  traverse_contents contents
