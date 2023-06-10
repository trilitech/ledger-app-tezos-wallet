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

let () =
  match Sys.argv with
  | [| _; "micheline"; m; ("nanos" | "nanosp") as model; dir |] ->
      let device = Gen_integration.Device.of_string_exn model in
      let m = int_of_string m in
      let fp_hex = open_out (dir ^ model ^ "/samples.hex") in
      let ppf_hex = Format.formatter_of_out_channel fp_hex in
      print_string "Generating Micheline samples";
      Seq.iteri
        (fun i (expr, _, (`Hex txt as hex)) ->
          if Gen_utils.micheline_too_large_or_too_deep expr then (
            print_string "!";
            flush stdout)
          else (
            print_string ".";
            flush stdout;
            Format.fprintf ppf_hex "%s@\n" txt;
            let fp =
              open_out (Format.asprintf "%s/%s/test_%03d.sh" dir model i)
            in
            let ppf = Format.formatter_of_out_channel fp in
            Gen_integration.gen_expect_test_sign_micheline_data ~device ppf hex;
            close_out fp))
        (Seq.take m Gen_micheline.hex);
      Format.fprintf ppf_hex "%!";
      print_newline ()
  | [| _; "operations"; m; ("nanos" | "nanosp") as model; dir |] ->
      let device = Gen_integration.Device.of_string_exn model in
      let m = int_of_string m in
      let fp_hex = open_out (dir ^ model ^ "/samples.hex") in
      let ppf_hex = Format.formatter_of_out_channel fp_hex in
      print_string "Generating Micheline samples";
      Seq.iteri
        (fun i (_op, _, (`Hex txt as hex)) ->
          print_string ".";
          flush stdout;
          Format.fprintf ppf_hex "%s@\n" txt;
          let fp =
            open_out (Format.asprintf "%s/%s/test_%03d.sh" dir model i)
          in
          let ppf = Format.formatter_of_out_channel fp in
          Gen_integration.gen_expect_test_sign_operation ~device ppf hex;
          close_out fp)
        (Seq.take m Gen_operations.hex);
      Format.fprintf ppf_hex "%!";
      print_newline ()
  | [| _; _; _m; "nanox"; _dir |] ->
      Format.eprintf "Actually, only nanos & nanox is supported for now.@.";
      exit 1
  | _ ->
      Format.eprintf
        "Usage: %s micheline <samples> <nanos|nanosp|nanox> <dir>@."
        Sys.executable_name;
      exit 1
