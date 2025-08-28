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

open Tezos_protocol_023_PtSeouLo
open Tezos_micheline
open Test_c_parser_utils

let rec pp_node ~wrap ppf (node : Protocol.Script_repr.node) =
  match node with
  | String (_, s) -> Format.fprintf ppf "%S" s
  | Bytes (_, bs) ->
      Format.fprintf ppf "0x%s"
        (String.uppercase_ascii (Hex.show (Hex.of_bytes bs)))
  | Int (_, n) -> Format.fprintf ppf "%s" (Z.to_string n)
  | Seq (_, l) ->
      Format.fprintf ppf "{%a}"
        (Format.pp_print_list
           ~pp_sep:(fun ppf () -> Format.fprintf ppf ";")
           (pp_node ~wrap:false))
        l
  | Prim (_, p, l, a) ->
      let lwrap, rwrap =
        if wrap && (l <> [] || a <> []) then ("(", ")") else ("", "")
      in
      Format.fprintf ppf "%s%s%a%s%s" lwrap
        (Protocol.Michelson_v1_primitives.string_of_prim p)
        (fun ppf l ->
          List.iter (fun e -> Format.fprintf ppf " %a" (pp_node ~wrap:true) e) l)
        l
        (if a = [] then "" else " " ^ String.concat " " a)
        rwrap

let to_string node =
  Format.asprintf "%a" (pp_node ~wrap:false) (Micheline.root node)

let to_bytes expr =
  Data_encoding.Binary.to_bytes_exn Protocol.Script_repr.expr_encoding expr

external cparse_step :
  cparse_state ->
  input:bytes * int * int ->
  output:bytes * int * int ->
  int * int * st = "micheline_cparse_step"
