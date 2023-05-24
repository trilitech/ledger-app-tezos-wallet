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

let path = path_to_bytes [ 44; 1729; 0; 0 ]

let _pkh =
  Tezos_crypto.Signature.Public_key_hash.of_b58check_exn
    "tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E"

let _pk =
  Tezos_crypto.Signature.Public_key.of_b58check_exn
    "edpkuXX2VdkdXzkN11oLCb8Aurdo1BTAtQiK8ZY9UPj2YMt3AHEpcY"

let sk =
  Tezos_crypto.Signature.Secret_key.of_b58check_exn
    "edsk2tUyhVvGj9B1S956ZzmaU4bC9J7t8xVBH52fkAoZL25MHEwacd"

module Class = struct
  type t = Default

  let to_uint8 = function Default -> 0x80
end

module Instruction = struct
  type t =
    | Version
    | Authorize_baking
    | Get_public_key
    | Prompt_public_key
    | Sign
    | Sign_unsafe
    | Reset
    | Query_auth_key
    | Query_main_hwm
    | Git
    | Setup
    | Query_all_hwm
    | Deauthorize
    | Query_auth_key_with_curve
    | Hmac
    | Sign_with_hash

  let to_uint8 = function
    | Version -> 0x00
    | Authorize_baking -> 0x01
    | Get_public_key -> 0x02
    | Prompt_public_key -> 0x03
    | Sign -> 0x04
    | Sign_unsafe -> 0x05
    | Reset -> 0x06
    | Query_auth_key -> 0x07
    | Query_main_hwm -> 0x08
    | Git -> 0x09
    | Setup -> 0x0A
    | Query_all_hwm -> 0x0B
    | Deauthorize -> 0x0C
    | Query_auth_key_with_curve -> 0x0D
    | Hmac -> 0x0E
    | Sign_with_hash -> 0x0F
end

module Index = struct
  type t = int

  let to_uint8 ?(is_last = false) idx =
    let is_last = if is_last then 0x80 else 0x00 in
    is_last lor idx
end

module Curve = struct
  type t = ED25519 | SECP256K1 | SECP256R1 | BIP32_ED25519

  let to_uint8 = function
    | ED25519 -> 0x00
    | SECP256K1 -> 0x01
    | SECP256R1 -> 0x02
    | BIP32_ED25519 -> 0x03
end

let apdu_success = Bytes.of_string "\x90\x00"
let max_apdu_size = 235

let make_packet ?is_last ?(idx = 0) ~cla ~ins ~curve content =
  let content_length = Bytes.length content in
  assert (content_length <= max_apdu_size);
  let header = Bytes.create 5 in
  Bytes.set_uint8 header 0 @@ Class.to_uint8 cla;
  Bytes.set_uint8 header 1 @@ Instruction.to_uint8 ins;
  Bytes.set_uint8 header 2 @@ Index.to_uint8 ?is_last idx;
  Bytes.set_uint8 header 3 @@ Curve.to_uint8 curve;
  Bytes.set_uint8 header 4 content_length;
  Bytes.cat header content

let make_packets ?(idx = 0) ~cla ~ins ~curve content =
  let len = Bytes.length content in
  let rec aux ofs idx =
    let remaining_size = len - ofs in
    if remaining_size <= max_apdu_size then
      let packet =
        make_packet ~is_last:true ~idx ~cla ~ins ~curve
        @@ Bytes.sub content ofs remaining_size
      in
      [ packet ]
    else
      let packet =
        make_packet ~idx ~cla ~ins ~curve @@ Bytes.sub content ofs max_apdu_size
      in
      packet :: aux (ofs + max_apdu_size) (idx + 1)
  in
  aux 0 idx

let split_sign_apdus bin =
  let cla = Class.Default in
  let ins = Instruction.Sign_with_hash in
  let curve = Curve.ED25519 in
  let apdus =
    make_packet ~cla ~ins ~curve path
    :: make_packets ~idx:1 ~cla ~ins ~curve bin
  in
  let expectations =
    let bin_sign_success_bytes =
      let bin_hash = Tezos_crypto.Blake2B.(to_bytes (hash_bytes [ bin ])) in
      let sign =
        Tezos_crypto.Signature.to_bytes (Tezos_crypto.Signature.sign sk bin)
      in
      Bytes.concat Bytes.empty [ bin_hash; sign; apdu_success ]
    in
    List.init (List.length apdus - 1) (fun _ -> apdu_success)
    @ [ bin_sign_success_bytes ]
  in
  List.combine apdus expectations

open Tezos_protocol_015_PtLimaPt
open Tezos_micheline

let decode_expr bin =
  Micheline.root
    (Data_encoding.Binary.of_bytes_exn Protocol.Script_repr.expr_encoding
       (Bytes.sub bin 1 (Bytes.length bin - 1)))

let decode_op bin =
  Data_encoding.Binary.of_bytes_exn
    Protocol.Alpha_context.Operation.unsigned_encoding
    (Bytes.sub bin 1 (Bytes.length bin - 1))

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

let split_screens size whole =
  let len = String.length whole in
  let rec split ofs acc =
    if len - ofs <= size then List.rev (String.sub whole ofs (len - ofs) :: acc)
    else split (ofs + size) (String.sub whole ofs size :: acc)
  in
  split 0 []

let pp_op size
    ( (_shell : Tezos_base.Operation.shell_header),
      (Contents_list contents : Protocol.Alpha_context.packed_contents_list) ) =
  let screens = ref [] in
  let push t fmt =
    Format.kasprintf (fun v -> screens := (t, v) :: !screens) fmt
  in
  let print_manager n (type t)
      (Manager_operation { fee; operation; storage_limit; _ } :
        t Protocol.Alpha_context.Kind.manager Protocol.Alpha_context.contents) =
    push
      (Format.asprintf "Operation (%d)" n)
      (match operation with
      | Transaction _ -> "Transaction"
      | _ -> "UNSUPPORTED");
    push "Fee" "%a tz" Protocol.Alpha_context.Tez.pp fee;
    push "Storage limit" "%s" (Z.to_string storage_limit);
    match operation with
    | Transaction { amount; entrypoint; destination; parameters } ->
        push "Amount" "%a tz" Protocol.Alpha_context.Tez.pp amount;
        push "Destination" "%a" Protocol.Alpha_context.Contract.pp destination;
        push "Entrypoint" "%a" Protocol.Alpha_context.Entrypoint.pp entrypoint;
        let node =
          let bin =
            Data_encoding.Binary.to_bytes_exn
              Protocol.Alpha_context.Script.lazy_expr_encoding parameters
          in
          Micheline.root
            (Data_encoding.Binary.of_bytes_exn
               Protocol.Script_repr.expr_encoding
               (Bytes.sub bin 4 (Bytes.length bin - 4)))
        in

        let whole = Format.asprintf "%a" (pp_node ~wrap:false) node in
        List.iter (push "Data" "%s") (split_screens size whole)
    | _ -> assert false
  in

  let rec print_op :
      type t. int -> t Protocol.Alpha_context.contents_list -> unit =
   fun n -> function
    | Single (Manager_operation _ as m) -> print_manager n m
    | Cons ((Manager_operation _ as m), rest) ->
        print_manager n m;
        print_op (succ n) rest
    | _ -> assert false
  in
  print_op 0 contents;
  List.rev !screens

let shell_escape ppf s =
  Format.fprintf ppf "'";
  String.iter
    (function
      | '\'' -> Format.fprintf ppf "'\\''" | c -> Format.fprintf ppf "%c" c)
    s;
  Format.fprintf ppf "'"

let gen_expect_test_sign ppf (`Hex txt as hex) screens =
  let bin = Hex.to_bytes hex in
  Format.fprintf ppf "# full input: %s@\n" txt;
  let screens = screens bin in
  Format.fprintf ppf "send_async_apdus";
  let apdus = split_sign_apdus bin in
  List.iter
    (fun (apdu, ans) ->
      Format.fprintf ppf "\\@\n  %a %a" Hex.pp (Hex.of_bytes apdu) Hex.pp
        (Hex.of_bytes ans))
    apdus;
  Format.fprintf ppf "@\n";
  List.iter
    (fun (t, s) ->
      Format.fprintf ppf "expect_full_text '%s' %a\npress_button right@\n" t
        shell_escape s)
    screens;
  Format.fprintf ppf "expect_full_text 'Accept?'@\n";
  Format.fprintf ppf "press_button both@\nexpect_async_apdus_sent@."

let gen_expect_test_sign_micheline_data ppf hex =
  let screens bin =
    let node = decode_expr bin in
    let whole = Format.asprintf "%a" (pp_node ~wrap:false) node in
    Format.fprintf ppf "# full output: %s@\n" whole;
    List.map (fun s -> ("Data", s)) (split_screens 38 whole)
  in
  gen_expect_test_sign ppf hex screens

let gen_expect_test_sign_operation ppf hex =
  let screens bin =
    let op = decode_op bin in
    pp_op 38 op
  in
  gen_expect_test_sign ppf hex screens
