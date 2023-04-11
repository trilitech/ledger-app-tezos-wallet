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

open Tezos_client_015_PtLimaPt

let preprocess_pattern_string p =
  let rewrite p =
    let rec try_metas = function
      | [] -> Stdlib.failwith "the pattern is bad"
      | (prefix, n) :: metas ->
          if String.starts_with ~prefix p then
            let l = String.length prefix in
            let def = String.sub p l (String.length p - l) in
            "(sapling_transaction_deprecated " ^ string_of_int n ^ def ^ ")"
          else try_metas metas
    in
    let rec try_caps metas = function
      | [] -> try_metas metas
      | (prefix, n) :: caps ->
          if String.starts_with ~prefix p then
            let l = String.length prefix in
            let name = String.sub p l (String.length p - l) in
            let name = String.trim name in
            "(sapling_transaction_deprecated " ^ string_of_int n ^ " \"" ^ name
            ^ "\")"
          else try_caps metas caps
    in
    try_caps
      [ ("list", 62); ("or", 63) ]
      [ ("any", 0); ("bytes", 1); ("int", 2); ("string", 3); ("address", 4) ]
  in
  let rec fix p =
    let p' =
      let b = Buffer.create 0 in
      Buffer.add_substitute b rewrite p;
      Buffer.contents b
    in
    if p = p' then p else fix p'
  in
  fix p

let read_pattern input =
  let input = preprocess_pattern_string input in
  let parsed =
    Tezos_micheline.Micheline_parser.no_parsing_error
      (Michelson_v1_parser.parse_expression input)
  in
  match parsed with
  | Error _ -> Stdlib.failwith "cannot parse pattern"
  | Ok { expanded; _ } -> expanded
