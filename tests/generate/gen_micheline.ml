open Tezos_protocol_015_PtLimaPt
open Tezos_micheline
open Tezos_benchmarks_proto_015_PtLimaPt

let st = Random.State.make_self_init ()

let config =
  {
    Michelson_generation.target_size = { min = 10; max = 100 };
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

let expr = Seq.append vector (Seq.concat (Seq.forever gen))

let bin =
  Seq.map
    (fun expr ->
      ( expr,
        Data_encoding.Binary.to_bytes_exn Protocol.Script_repr.expr_encoding
          expr ))
    expr

let hex =
  Seq.map
    (fun (expr, bin) ->
      (expr, bin, `Hex (String.cat "05" (Hex.show (Hex.of_bytes bin)))))
    bin
