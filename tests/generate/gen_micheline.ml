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

open Tezos_protocol_017_PtNairob
open Tezos_micheline
open Tezos_benchmarks_proto_017_PtNairob

let config =
  {
    Michelson_generation.target_size = { min = 10; max = 100 };
    burn_in_multiplier = 1;
  }

(* Only use this function to generate values *)
let gen_code =
  let gen rand = (Michelson_generation.make_code_sampler rand config).term in
  let shrink _ = assert false in
  QCheck2.Gen.make_primitive ~gen ~shrink

(* Only use this function to generate values *)
let gen_data =
  let gen rand = (Michelson_generation.make_data_sampler rand config).term in
  let shrink _ = assert false in
  QCheck2.Gen.make_primitive ~gen ~shrink

let gen_expr = QCheck2.Gen.oneof [ gen_data; gen_code ]

let gen () =
  List.to_seq
    [
      QCheck2.Gen.generate1 ~rand:Gen_utils.random_state gen_code;
      QCheck2.Gen.generate1 ~rand:Gen_utils.random_state gen_data;
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
let watermark = Tezos_crypto.Signature.Custom (Bytes.of_string "\x05")

let encode expr =
  Data_encoding.Binary.to_bytes_exn Protocol.Script_repr.expr_encoding expr

let decode bin =
  Data_encoding.Binary.of_bytes_exn Protocol.Script_repr.expr_encoding bin

let hex =
  Seq.map
    (fun expr ->
      let bin = encode expr in
      let hex = Hex.of_bytes bin in
      (expr, bin, hex))
    expr
