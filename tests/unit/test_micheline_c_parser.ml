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

open Tezos_protocol_015_PtLimaPt
open Tezos_micheline

let pp_ocaml node =
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
            List.iter
              (fun e -> Format.fprintf ppf " %a" (pp_node ~wrap:true) e)
              l)
          l
          (if a = [] then "" else " " ^ String.concat " " a)
          rwrap
  in
  Format.asprintf "%a" (pp_node ~wrap:false) (Micheline.root node)

type cparse_state

[@@@ocaml.warning "-37"]

external cparse_init : bytes option -> int -> cparse_state
  = "micheline_cparse_init"

type st = DONE | FEED_ME | IM_FULL

external micheline_cparse_step :
  cparse_state ->
  input:bytes * int * int ->
  output:bytes * int * int ->
  int * int * st = "micheline_cparse_step"

let pp_c_bin ppf bytes =
  let len = Bytes.length bytes in
  let state = cparse_init None len in
  let ss = 1 + Random.int 1000 in
  let rec screen_by_screen ss ofs tr tw sn =
    let buf = Bytes.make ss ' ' in
    let read, written, st =
      micheline_cparse_step state
        ~input:(bytes, ofs, len - ofs)
        ~output:(buf, 0, ss)
    in
    let ofs = ofs + read in
    let tr = tr + read in
    let tw = tw + written in
    if written > 0 then
      Format.fprintf ppf "%s" (Bytes.to_string (Bytes.sub buf 0 written));
    match st with
    | FEED_ME -> ()
    | IM_FULL ->
        screen_by_screen ss ofs tr tw (if written > 0 then sn + 1 else sn)
    | DONE -> ()
  in
  screen_by_screen ss 0 0 0 0

let pp_c expr =
  let bin =
    Data_encoding.Binary.to_bytes_exn Protocol.Script_repr.expr_encoding expr
  in
  try Format.asprintf "%a" pp_c_bin bin with exn -> Printexc.to_string exn

let check (nfail, nok, failed) expr =
  if (nfail + nok) mod 100 = 0 then
    Format.printf "Running: %d (%d failed)...%s%!" (nfail + nok) nfail
      (String.make 80 '\b');
  let exp = pp_ocaml expr in
  let res = pp_c expr in
  if exp = res then (nfail, nok + 1, failed)
  else (nfail + 1, nok, (exp, res) :: failed)
