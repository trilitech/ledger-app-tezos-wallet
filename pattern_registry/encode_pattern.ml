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

let preprocess_pattern input =
  let input = Process_pattern.read_pattern input in
  let bin =
    Data_encoding.Binary.to_bytes_exn
      Tezos_protocol_015_PtLimaPt.Protocol.Script_repr.expr_encoding input
  in
  Format.printf "%a" Hex.pp (Hex.of_bytes bin)

let () = preprocess_pattern Sys.argv.(1)
