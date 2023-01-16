open Tezos_protocol_015_PtLimaPt
open Tezos_micheline
open Tezos_benchmarks_proto_015_PtLimaPt

let st = Random.State.make_self_init ()

let config =
  {
    Michelson_generation.target_size = { min = 0; max = 10 };
    burn_in_multiplier = 1;
  }

let gen () =
  List.to_seq
    [
      (Michelson_generation.make_code_sampler st config).term;
      (Michelson_generation.make_data_sampler st config).term;
    ]

let vector =
  List.to_seq
    (List.map Micheline.strip_locations
       ([ Micheline.Prim (0, Protocol.Michelson_v1_primitives.D_Unit, [], []) ]
       @ List.map
           (fun n -> Micheline.Int (0, Z.of_string n))
           (List.flatten
              (List.map
                 (fun s -> [ s; "-" ^ s ])
                 [
                   "0";
                   "1";
                   "1234567890";
                   "12345678901234567890";
                   "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
                   "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
                 ]))))

let () =
  assert (Array.length Sys.argv = 3);
  let m = int_of_string Sys.argv.(2) in
  let fp = if Sys.argv.(1) = "-" then stdout else open_out Sys.argv.(1) in
  let output expr =
    let bin =
      Data_encoding.Binary.to_bytes_exn Protocol.Script_repr.expr_encoding expr
    in
    output_string fp "05";
    output_string fp (Hex.show (Hex.of_bytes bin));
    output_string fp "\n"
  in
  Seq.iter output
    (Seq.take m (Seq.append vector (Seq.concat (Seq.forever gen))));
  close_out fp
