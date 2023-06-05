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

type cparse_state

[@@@ocaml.warning "-37"]

external cparse_init : bytes option -> int -> cparse_state
  = "micheline_cparse_init"

type st = DONE | FEED_ME | IM_FULL

type cparse_step =
  cparse_state ->
  input:bytes * int * int ->
  output:bytes * int * int ->
  int * int * st
