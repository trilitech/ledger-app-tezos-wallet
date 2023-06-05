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

let () =
  match Sys.argv with
  | [| _; "micheline"; path |] ->
      let fp = if path = "-" then stdin else open_in path in
      let inputs =
        Seq.of_dispenser (fun () ->
            try
              let bin = Hex.to_string (`Hex (input_line fp)) in
              let _, expr =
                Data_encoding.Binary.read_exn Protocol.Script_repr.expr_encoding
                  bin 1
                  (String.length bin - 1)
              in
              Some expr
            with End_of_file ->
              close_in fp;
              None)
      in
      let nfail, nok, failed =
        Seq.fold_left Test_micheline_c_parser.check (0, 0, []) inputs
      in
      let ntoo_deep, ntoo_large, failed =
        List.fold_left
          (fun (acctoo_deep, acctoo_large, accl) (exp, res) ->
            if res = "Failure(\"micheline_cparse_step: expression too deep\")"
            then (acctoo_deep + 1, acctoo_large, accl)
            else if
              res
              = "Failure(\"micheline_cparse_step: data size limitation \
                 exceeded\")"
            then (acctoo_deep, acctoo_large + 1, accl)
            else (acctoo_deep, acctoo_large, (exp, res) :: accl))
          (0, 0, []) failed
      in
      Format.printf
        "Result: %d test were run, %d hard failed, %d failed with TOO_DEEP, %d \
         failed with TOO_LARGE.@."
        (nfail + nok)
        (nfail - ntoo_deep - ntoo_large)
        ntoo_deep ntoo_large;
      List.iter
        (fun (exp, res) ->
          Format.printf "%s@.Expected: %S@.Got: %S@."
            (String.make
               (match Terminal_size.get_columns () with
               | Some v -> v
               | None -> 80)
               '-')
            exp res)
        failed
  | _ ->
      Format.eprintf "Usage: %s micheline <path>@." Sys.executable_name;
      exit 1
