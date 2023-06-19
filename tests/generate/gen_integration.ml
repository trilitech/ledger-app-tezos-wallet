(* Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
   Copyright 2023 Functori <contact@functori.com>
   Copyright 2023 TriliTech <contact@trili.tech>

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. *)

let shell_escape ppf s =
  Format.fprintf ppf "'";
  String.iter
    (function
      | '\'' -> Format.fprintf ppf "'\\''" | c -> Format.fprintf ppf "%c" c)
    s;
  Format.fprintf ppf "'"

let pp_hex_bytes ppf bytes = Format.fprintf ppf "%a" Hex.pp (Hex.of_bytes bytes)

(** General *)
module Device = struct
  type t = Nanos | Nanosp | Nanox

  let of_string_exn = function
    | "nanos" -> Nanos
    | "nanosp" -> Nanosp
    | "nanox" -> Nanox
    | d -> failwith @@ Format.sprintf "invalid device %s" d

  let to_string = function
    | Nanos -> "nanos"
    | Nanosp -> "nanosp"
    | Nanox -> "nanox"
end

let expect_full_text ppf l =
  let pp_space ppf () = Format.fprintf ppf " " in
  let pp_args = Format.pp_print_list ~pp_sep:pp_space shell_escape in
  Format.fprintf ppf "expect_full_text %a@." pp_args l

let expect_section_content ppf ~device ~title content =
  Format.fprintf ppf "expect_section_content %s %a %a@."
    (Device.to_string device) shell_escape title shell_escape content

module Button = struct
  type t = Both | Right | Left

  let press ppf button =
    let button =
      match button with Both -> "both" | Right -> "right" | Left -> "left"
    in
    Format.fprintf ppf "press_button %s@." button
end

let send_apdu ppf packet =
  Format.fprintf ppf "send_apdu %a@." pp_hex_bytes packet

let expect_apdu_return ppf ans =
  Format.fprintf ppf "expect_apdu_return %a@." pp_hex_bytes ans

type async_apdu = { packet : bytes; check : Format.formatter -> unit -> unit }

let send_async_apdus ppf async_apdus =
  let pp_dash_break_line ppf () = Format.fprintf ppf "\\@\n\t" in
  let pp_async_apdu ppf { packet; check } =
    Format.fprintf ppf "%a \"%a\"" pp_hex_bytes packet check ()
  in
  let pp_async_apdus =
    Format.pp_print_list ~pp_sep:pp_dash_break_line pp_async_apdu
  in
  Format.fprintf ppf "send_async_apdus %a%a@." pp_dash_break_line ()
    pp_async_apdus async_apdus

let expect_async_apdus_sent ppf () =
  Format.fprintf ppf "expect_async_apdus_sent@."

let expect_exited ppf () = Format.fprintf ppf "expect_exited@."

let check_tlv_signature_from_sent_apdu ppf ~prefix ~suffix pk message =
  Format.fprintf ppf "check_tlv_signature_from_sent_apdu %a %a %a %a@."
    pp_hex_bytes prefix pp_hex_bytes suffix Tezos_crypto.Signature.Public_key.pp
    pk pp_hex_bytes message

(** Specific *)

let home ppf () =
  expect_full_text ppf [ "Tezos Wallet"; "ready for"; "safe signing" ]

let expect_accept ppf = function
  | Device.Nanos -> expect_full_text ppf [ "Accept?" ]
  | Device.Nanosp | Device.Nanox ->
      expect_full_text ppf [ "Accept?"; "Press both buttons"; "to accept." ]

let expect_reject ppf = function
  | Device.Nanos -> expect_full_text ppf [ "Reject?" ]
  | Device.Nanosp | Device.Nanox ->
      expect_full_text ppf [ "Accept?"; "Press both buttons"; "to reject." ]

let expect_quit ppf () = expect_full_text ppf [ "Quit?" ]

let accept ppf device =
  expect_accept ppf device;
  Button.(press ppf Both)

let reject ppf device =
  expect_reject ppf device;
  Button.(press ppf Both)

let quit ppf () =
  expect_quit ppf ();
  Button.(press ppf Both)

type screen = { title : string; contents : string list; multi : bool }

let make_screen ~title ?(multi = false) contents = { title; contents; multi }

let expected_screen ppf ~device { title; contents; multi } =
  if multi then expect_section_content ppf ~device ~title @@ List.hd contents
  else expect_full_text ppf (title :: contents)

let go_through_screens ppf ~device screens =
  List.iter
    (fun screen ->
      expected_screen ppf ~device screen;
      Button.(press ppf Right))
    screens

let sign ppf ~signer:Apdu.Signer.({ sk; pk; _ } as signer) bin =
  let packets = Apdu.sign ~signer bin in
  let bin_accept_check ppf () =
    let bin_hash = Tezos_crypto.Blake2B.(to_bytes (hash_bytes [ bin ])) in
    if Apdu.Curve.(deterministic_sig (of_sk sk)) then
      let sign =
        Tezos_crypto.Signature.to_bytes (Tezos_crypto.Signature.sign sk bin)
      in
      expect_apdu_return ppf
        (Bytes.concat Bytes.empty [ bin_hash; sign; Apdu.success ])
    else
      check_tlv_signature_from_sent_apdu ppf ~prefix:bin_hash
        ~suffix:Apdu.success pk bin
  in
  let last_index = List.length packets - 1 in
  let async_apdus =
    List.mapi
      (fun index packet ->
        let check ppf () =
          if index = last_index then bin_accept_check ppf ()
          else expect_apdu_return ppf Apdu.success
        in
        { packet; check })
      packets
  in
  send_async_apdus ppf async_apdus

open Tezos_protocol_016_PtMumbai
open Tezos_micheline

let rec pp_node ~wrap ppf (node : Protocol.Script_repr.node) =
  match node with
  | String (_, s) -> Format.fprintf ppf "%S" s
  | Bytes (_, bs) ->
      Format.fprintf ppf "0x%s"
        (String.uppercase_ascii (Hex.show (Hex.of_bytes bs)))
  | Int (_, n) -> Format.fprintf ppf "%s" (Z.to_string n)
  | Seq (_, l) ->
      Format.fprintf ppf "{%a}"
        (Format.pp_print_list
           ~pp_sep:(fun ppf () -> Format.fprintf ppf ";")
           (pp_node ~wrap:false))
        l
  | Prim (_, p, l, a) ->
      let lwrap, rwrap =
        if wrap && (l <> [] || a <> []) then ("(", ")") else ("", "")
      in
      Format.fprintf ppf "%s%s%a%s%s" lwrap
        (Protocol.Michelson_v1_primitives.string_of_prim p)
        (fun ppf l ->
          List.iter (fun e -> Format.fprintf ppf " %a" (pp_node ~wrap:true) e) l)
        l
        (if a = [] then "" else " " ^ String.concat " " a)
        rwrap

let screen_width = 19
let nanos_screen_lines = 3

let split_screens ~title whole =
  let screen_data_lines = nanos_screen_lines - 1 in
  let max_size = screen_data_lines * screen_width in
  let len = String.length whole in
  let rec split ofs acc =
    let remaining_size = len - ofs in
    if remaining_size <= max_size then
      let content = String.sub whole ofs remaining_size in
      let screen = make_screen ~title [ content ] in
      screen :: acc
    else
      let content = String.sub whole ofs max_size in
      let screen = make_screen ~title [ content ] in
      split (ofs + max_size) (screen :: acc)
  in
  List.rev @@ split 0 []

let node_to_screens ppf node =
  let whole = Format.asprintf "%a" (pp_node ~wrap:false) node in
  Format.fprintf ppf "# full output: %s@\n" whole;
  [ make_screen ~title:"Data" ~multi:true [ whole ] ]

let operation_to_screens ppf
    ( (_shell : Tezos_base.Operation.shell_header),
      (Contents_list contents : Protocol.Alpha_context.packed_contents_list) ) =
  let open Protocol.Alpha_context in
  let make_screen ~title fmt =
    Format.kasprintf (fun content -> make_screen ~title [ content ]) fmt
  in
  let screen_of_manager n (type t)
      (Manager_operation { fee; operation; storage_limit; _ } :
        t Kind.manager contents) =
    match operation with
    | Transaction { amount; entrypoint; destination; parameters } ->
        let operation_title = Format.asprintf "Operation (%d)" n in
        let operation_screen =
          make_screen ~title:operation_title "Transaction"
        in
        let fee_screen = make_screen ~title:"Fee" "%a tz" Tez.pp fee in
        let storage_screen =
          make_screen ~title:"Storage limit" "%s" (Z.to_string storage_limit)
        in
        let amount_screen = make_screen ~title:"Amount" "%a tz" Tez.pp amount in
        let destination_screen =
          make_screen ~title:"Destination" "%a" Contract.pp destination
        in
        let entrypoint_screen =
          make_screen ~title:"Entrypoint" "%a" Entrypoint.pp entrypoint
        in
        let node =
          let expr =
            Result.get_ok @@ Protocol.Script_repr.force_decode parameters
          in
          Micheline.root expr
        in
        let node_screens = node_to_screens ppf node in
        operation_screen :: fee_screen :: storage_screen :: amount_screen
        :: destination_screen :: entrypoint_screen :: node_screens
    | _ -> assert false
  in
  let rec screen_of_operations : type t. int -> t contents_list -> screen list =
   fun n -> function
    | Single (Manager_operation _ as m) -> screen_of_manager n m
    | Cons ((Manager_operation _ as m), rest) ->
        screen_of_manager n m @ screen_of_operations (succ n) rest
    | _ -> assert false
  in
  screen_of_operations 0 contents

(* Keys for mnemonic zebra (x24), path m/44'/1729'/0'/0' *)

let tz1_signer =
  (* tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E *)
  Apdu.Signer.make ~path:[ 44; 1729; 0; 0 ]
    ~sk:"edsk2tUyhVvGj9B1S956ZzmaU4bC9J7t8xVBH52fkAoZL25MHEwacd"

let tz2_signer =
  (* tz2GB5YHqF4UzQ8GP5yUqdhY9oVWRXCY2hPU *)
  Apdu.Signer.make ~path:[ 44; 1729; 0; 0 ]
    ~sk:"spsk2Pfx9chqXVbz2tW7ze4gGU4RfaiK3nSva77bp69zHhFho2zTze"

let gen_signer = QCheck2.Gen.oneofl [ tz1_signer ]

let gen_expect_test_sign ppf (`Hex txt as hex) ~device screens =
  let bin = Hex.to_bytes hex in
  Format.fprintf ppf "# full input: %s@." txt;
  let screens = screens bin in
  let signer = QCheck2.Gen.generate1 ~rand:Gen_utils.random_state gen_signer in
  Format.fprintf ppf "# signer: %a@." Tezos_crypto.Signature.Public_key_hash.pp
    signer.pkh;
  home ppf ();
  sign ppf ~signer bin;
  go_through_screens ppf ~device screens;
  accept ppf device;
  expect_async_apdus_sent ppf ()

let gen_expect_test_sign_micheline_data ~device ppf hex =
  let screens bin =
    let node = Gen_micheline.decode bin in
    node_to_screens ppf node
  in
  gen_expect_test_sign ppf hex ~device screens

let gen_expect_test_sign_operation ~device ppf hex =
  let screens bin =
    let op = Gen_operations.decode bin in
    operation_to_screens ppf op
  in
  gen_expect_test_sign ppf ~device hex screens
