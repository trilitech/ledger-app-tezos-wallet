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

open Tezos_micheline

let random_state = Random.State.make_self_init ()

let micheline_too_large_or_too_deep expr =
  try
    let rec traverse d expr =
      if d > 40 then raise Exit
      else
        match expr with
        | Micheline.Int (_, n) -> if Z.size n * 64 > 256 then raise Exit
        | Micheline.String _ | Micheline.Bytes _ -> ()
        | Micheline.Prim (_, _, n, _) | Micheline.Seq (_, n) ->
            List.iter (traverse (d + 1)) n
    in
    traverse 0 (Micheline.root expr);
    false
  with Exit -> true
