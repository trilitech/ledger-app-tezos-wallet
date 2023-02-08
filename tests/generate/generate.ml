let () =
  match Sys.argv with
  | [| _; "micheline"; m; "nanos" as model; dir |] ->
      let m = int_of_string m in
      let fp_hex = open_out (dir ^ "/micheline_samples.hex") in
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
              open_out (Format.asprintf "%s/test_%s_%03d.sh" dir model i)
            in
            let ppf = Format.formatter_of_out_channel fp in
            Gen_integration.gen_expect_test ppf hex;
            close_out fp))
        (Seq.take m Gen_micheline.hex);
      Format.fprintf ppf_hex "%!";
      print_newline ()
  | [| _; "micheline"; _m; "nanosp" | "nanox"; _dir |] ->
      Format.eprintf "Actually, only nanos is supported for now.@.";
      exit 1
  | _ ->
      Format.eprintf
        "Usage: %s micheline <samples> <nanos|nanosp|nanox> <dir>@."
        Sys.executable_name;
      exit 1
