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

let shell_escape ppf s =
  Format.fprintf ppf "'";
  String.iter
    (function
      | '\'' -> Format.fprintf ppf "'\\''" | c -> Format.fprintf ppf "%c" c)
    s;
  Format.fprintf ppf "'"

let pp_hex_bytes ppf bytes = Format.fprintf ppf "%a" Hex.pp (Hex.of_bytes bytes)

(** General *)

let expect_full_text ppf l =
  let pp_space ppf () = Format.fprintf ppf " " in
  let pp_args = Format.pp_print_list ~pp_sep:pp_space shell_escape in
  Format.fprintf ppf "expect_full_text %a@." pp_args l

module Button = struct
  type t = Both | Right | Left

  let press ppf button =
    let button =
      match button with Both -> "both" | Right -> "right" | Left -> "left"
    in
    Format.fprintf ppf "press_button %s@." button
end

let send_apdu ppf apdu = Format.fprintf ppf "send_apdu %a@." pp_hex_bytes apdu

let expect_apdu_return ppf ans =
  Format.fprintf ppf "expect_apdu_return %a@." pp_hex_bytes ans

type async_apdu = { apdu : bytes; check : Format.formatter -> unit -> unit }

let send_async_apdus ppf apdus =
  let pp_dash_break_line ppf () = Format.fprintf ppf "\\@\n\t" in
  let pp_apdu ppf { apdu; check } =
    Format.fprintf ppf "%a \"%a\"" pp_hex_bytes apdu check ()
  in
  let pp_apdus = Format.pp_print_list ~pp_sep:pp_dash_break_line pp_apdu in
  Format.fprintf ppf "send_async_apdus %a%a@." pp_dash_break_line () pp_apdus
    apdus

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

let expect_accept ppf () = expect_full_text ppf [ "Accept?" ]
let expect_reject ppf () = expect_full_text ppf [ "Reject?" ]
let expect_quit ppf () = expect_full_text ppf [ "Quit?" ]

let accept ppf () =
  expect_accept ppf ();
  Button.(press ppf Both)

let reject ppf () =
  expect_reject ppf ();
  Button.(press ppf Both)

let quit ppf () =
  expect_quit ppf ();
  Button.(press ppf Both)

type screen = { title : string; contents : string list }

let make_screen ~title contents = { title; contents }

let expected_screen ppf { title; contents } =
  expect_full_text ppf (title :: contents)

let go_through_screens ppf screens =
  List.iter
    (fun screen ->
      expected_screen ppf screen;
      Button.(press ppf Right))
    screens

type signer = {
  path : bytes;
  pkh : Tezos_crypto.Signature.public_key_hash;
  pk : Tezos_crypto.Signature.public_key;
  sk : Tezos_crypto.Signature.secret_key;
}

let sign_apdus ~signer:{ path; pkh = _; pk = _; sk } bin =
  let cla = Apdu.Class.Default in
  let ins = Apdu.Instruction.Sign_with_hash in
  let curve = Apdu.Curve.of_sk sk in
  Apdu.make_packet ~cla ~ins ~curve path
  :: Apdu.make_packets ~idx:1 ~cla ~ins ~curve bin

let sign ppf ~signer bin =
  let apdus = sign_apdus ~signer bin in
  let bin_accept_check ppf () =
    let bin_hash = Tezos_crypto.Blake2B.(to_bytes (hash_bytes [ bin ])) in
    if Apdu.Curve.(deterministic_sig (of_sk signer.sk)) then
      let sign =
        Tezos_crypto.Signature.to_bytes
          (Tezos_crypto.Signature.sign signer.sk bin)
      in
      expect_apdu_return ppf
        (Bytes.concat Bytes.empty [ bin_hash; sign; Apdu.success ])
    else
      check_tlv_signature_from_sent_apdu ppf ~prefix:bin_hash
        ~suffix:Apdu.success signer.pk bin
  in
  let last_index = List.length apdus - 1 in
  let async_apdus =
    List.mapi
      (fun index apdu ->
        let check ppf () =
          if index = last_index then bin_accept_check ppf ()
          else expect_apdu_return ppf Apdu.success
        in
        { apdu; check })
      apdus
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
  split_screens ~title:"Data" whole

let operation_to_screens ppf
    ( (_shell : Tezos_base.Operation.shell_header),
      (Contents_list contents : Protocol.Alpha_context.packed_contents_list) ) =
  let make_screen ~title fmt =
    Format.kasprintf (fun content -> make_screen ~title [ content ]) fmt
  in
  let screen_of_manager n (type t)
      (Manager_operation { fee; operation; storage_limit; _ } :
        t Protocol.Alpha_context.Kind.manager Protocol.Alpha_context.contents) =
    match operation with
    | Transaction { amount; entrypoint; destination; parameters } ->
        let operation_title = Format.asprintf "Operation (%d)" n in
        let operation_screen =
          make_screen ~title:operation_title "Transaction"
        in
        let fee_screen =
          make_screen ~title:"Fee" "%a tz" Protocol.Alpha_context.Tez.pp fee
        in
        let storage_screen =
          make_screen ~title:"Storage limit" "%s" (Z.to_string storage_limit)
        in
        let amount_screen =
          make_screen ~title:"Amount" "%a tz" Protocol.Alpha_context.Tez.pp
            amount
        in
        let destination_screen =
          make_screen ~title:"Destination" "%a"
            Protocol.Alpha_context.Contract.pp destination
        in
        let entrypoint_screen =
          make_screen ~title:"Entrypoint" "%a"
            Protocol.Alpha_context.Entrypoint.pp entrypoint
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
  let rec screen_of_operations :
      type t. int -> t Protocol.Alpha_context.contents_list -> screen list =
   fun n -> function
    | Single (Manager_operation _ as m) -> screen_of_manager n m
    | Cons ((Manager_operation _ as m), rest) ->
        screen_of_manager n m @ screen_of_operations (succ n) rest
    | _ -> assert false
  in
  screen_of_operations 0 contents

let path_to_bytes path =
  (* hardened is defined by the "'" *)
  let hardened idx = idx lor 0x80_00_00_00 in
  let length = List.length path in
  let bytes = Bytes.create ((4 * length) + 1) in
  Bytes.set_uint8 bytes 0 length;
  List.iteri
    (fun i idx ->
      Bytes.set_int32_be bytes ((4 * i) + 1) (Int32.of_int (hardened idx)))
    path;
  bytes

(* Keys for mnemonic zebra (x24), path m/44'/1729'/0'/0' *)

let tz1_signer =
  (* tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E *)
  let open Tezos_crypto.Signature in
  let path = path_to_bytes [ 44; 1729; 0; 0 ] in
  let sk =
    Secret_key.of_b58check_exn
      "edsk2tUyhVvGj9B1S956ZzmaU4bC9J7t8xVBH52fkAoZL25MHEwacd"
  in
  let pk = Secret_key.to_public_key sk in
  let pkh = Public_key.hash pk in
  { path; pkh; pk; sk }

let tz2_signer =
  (* tz2GB5YHqF4UzQ8GP5yUqdhY9oVWRXCY2hPU *)
  let open Tezos_crypto.Signature in
  let path = path_to_bytes [ 44; 1729; 0; 0 ] in
  let sk =
    Secret_key.of_b58check_exn
      "spsk2Pfx9chqXVbz2tW7ze4gGU4RfaiK3nSva77bp69zHhFho2zTze"
  in
  let pk = Secret_key.to_public_key sk in
  let pkh = Public_key.hash pk in
  { path; pkh; pk; sk }

let gen_signer = QCheck2.Gen.oneofl [ tz1_signer; tz2_signer ]

let gen_expect_test_sign ppf (`Hex txt as hex) screens =
  let bin = Hex.to_bytes hex in
  Format.fprintf ppf "# full input: %s@." txt;
  let screens = screens bin in
  let signer = QCheck2.Gen.generate1 ~rand:Gen_utils.random_state gen_signer in
  Format.fprintf ppf "# signer: %a@." Tezos_crypto.Signature.Public_key_hash.pp
    signer.pkh;
  sign ppf ~signer bin;
  go_through_screens ppf screens;
  accept ppf ();
  expect_async_apdus_sent ppf ()

let gen_expect_test_sign_micheline_data ppf hex =
  let screens bin =
    let node = Gen_micheline.decode bin in
    node_to_screens ppf node
  in
  gen_expect_test_sign ppf hex screens

let gen_expect_test_sign_operation ppf hex =
  let screens bin =
    let op = Gen_operations.decode bin in
    operation_to_screens ppf op
  in
  gen_expect_test_sign ppf hex screens
