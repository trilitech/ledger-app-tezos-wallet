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

open Tezos_protocol_018_Proxford
open Test_c_parser_utils

let read_hex_file ~encoding path =
  let fp = if path = "-" then stdin else open_in path in
  Seq.of_dispenser @@ fun () ->
  try
    let bin = Hex.to_string (`Hex (input_line fp)) in
    Some (Data_encoding.Binary.of_string_exn encoding bin)
  with End_of_file ->
    close_in fp;
    None

let pp_c_bin ~(cparse_step : cparse_step) ppf bytes =
  let len = Bytes.length bytes in
  let state = cparse_init len in
  let rec screen_by_screen ofs =
    let input_len = 1 + Random.int 234 in
    let output_len = 1 + Random.int 100 in
    let buf = Bytes.make output_len ' ' in
    let read, written, st =
      cparse_step state
        ~input:(bytes, ofs, min input_len (len - ofs))
        ~output:(buf, 0, output_len)
    in
    if written > 0 then
      Format.fprintf ppf "%s" (Bytes.to_string (Bytes.sub buf 0 written));
    match st with
    | FEED_ME -> if len - ofs > 0 then screen_by_screen (ofs + read)
    | IM_FULL -> screen_by_screen (ofs + read)
    | DONE -> ()
  in
  screen_by_screen 0

let pp_c ~to_bytes ~cparse_step input =
  try Format.asprintf "%a" (pp_c_bin ~cparse_step) @@ to_bytes input
  with exn -> Printexc.to_string exn

let check ~to_string ~to_bytes ~cparse_step inputs =
  let aux (nfail, nok, failed) i input =
    if i mod 100 = 0 then Format.printf "Running: %d (%d failed)...@." i nfail;
    let expected = to_string input in
    let got = pp_c ~to_bytes ~cparse_step input in
    if expected = got then (nfail, nok + 1, failed)
    else (nfail + 1, nok, (i, expected, got) :: failed)
  in
  Seq.fold_lefti aux (0, 0, []) inputs

let divider =
  let columns = Option.value ~default:80 @@ Terminal_size.get_columns () in
  String.make columns '-'

let display_failed =
  List.iter @@ fun (i, exp, res) ->
  Format.printf "%s@.Test: %i@.Expected: %S@.Got: %S@." divider i exp res

let () =
  match Sys.argv with
  | [| _; "micheline"; path |] ->
      let inputs =
        read_hex_file ~encoding:Protocol.Script_repr.expr_encoding path
      in
      let nfail, nok, failed =
        Test_micheline_c_parser.(check ~to_string ~to_bytes ~cparse_step inputs)
      in
      let ntoo_deep, ntoo_large, failed =
        List.fold_left
          (fun (acctoo_deep, acctoo_large, accl) ((_, _, res) as failed) ->
            if res = "Failure(\"micheline_cparse_step: expression too deep\")"
            then (acctoo_deep + 1, acctoo_large, accl)
            else if
              res
              = "Failure(\"micheline_cparse_step: data size limitation \
                 exceeded\")"
            then (acctoo_deep, acctoo_large + 1, accl)
            else (acctoo_deep, acctoo_large, failed :: accl))
          (0, 0, []) failed
      in
      display_failed failed;
      Format.printf
        "Result: %d test were run, %d hard failed, %d failed with TOO_DEEP, %d \
         failed with TOO_LARGE.@."
        (nfail + nok)
        (nfail - ntoo_deep - ntoo_large)
        ntoo_deep ntoo_large
  | [| _; "operations"; path |] ->
      let inputs =
        read_hex_file
          ~encoding:Protocol.Alpha_context.Operation.unsigned_encoding path
      in
      let nfail, nok, failed =
        Test_operations_c_parser.(
          check ~to_string ~to_bytes ~cparse_step inputs)
      in
      display_failed failed;
      Format.printf "Result: %d test were run, %d failed.@." (nfail + nok) nfail
  | _ ->
      Format.eprintf "Usage: %s <micheline|operations> <path>@."
        Sys.executable_name;
      exit 1
