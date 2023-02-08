(* Keys for mnemonic zebra (x24), path m/44'/1729'/0'/0' *)

let _pkh =
  Tezos_crypto.Signature.Public_key_hash.of_b58check_exn
    "tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E"

let _pk =
  Tezos_crypto.Signature.Public_key.of_b58check_exn
    "edpkuXX2VdkdXzkN11oLCb8Aurdo1BTAtQiK8ZY9UPj2YMt3AHEpcY"

let sk =
  Tezos_crypto.Signature.Secret_key.of_b58check_exn
    "edsk2tUyhVvGj9B1S956ZzmaU4bC9J7t8xVBH52fkAoZL25MHEwacd"

let split_sign_apdus bin =
  let len = Bytes.length bin in
  let rec split ofs idx =
    if ofs = len then []
    else
      let size = min (len - ofs) 235 in
      let last = ofs + size = len in
      let packet = Bytes.create (size + 5) in
      Bytes.set_uint8 packet 0 (* CLA *) 0x80;
      Bytes.set_uint8 packet 1 (* INS *) 0x0F;
      Bytes.set_uint8 packet 2 (* P1 *) ((if last then 0x80 else 0x00) lor idx);
      Bytes.set_uint8 packet 3 (* P2 *) 0x00;
      Bytes.set_uint8 packet 4 (* LC *) size;
      Bytes.blit bin ofs packet 5 size;
      let result =
        if last then
          Bytes.concat Bytes.empty
            [
              Tezos_crypto.Blake2B.(to_bytes (hash_bytes [ bin ]));
              Tezos_crypto.Signature.to_bytes
                (Tezos_crypto.Signature.sign sk bin);
              Bytes.of_string "\x90\x00";
            ]
        else Bytes.of_string "\x90\x00"
      in
      (packet, result) :: split (ofs + size) (idx + 1)
  in
  [
    ( Bytes.of_string
        "\x80\x0f\x00\x00\x11\x04\x80\x00\x00\x2c\x80\x00\x06\xc1\x80\x00\x00\x00\x80\x00\x00\x00",
      Bytes.of_string "\x90\x00" );
  ]
  @ split 0 1

open Tezos_protocol_015_PtLimaPt
open Tezos_micheline

let decode_expr bin =
  Micheline.root
    (Data_encoding.Binary.of_bytes_exn Protocol.Script_repr.expr_encoding
       (Bytes.sub bin 1 (Bytes.length bin - 1)))

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

let shell_escape ppf s =
  Format.fprintf ppf "'";
  String.iter
    (function
      | '\'' -> Format.fprintf ppf "'\\''" | c -> Format.fprintf ppf "%c" c)
    s;
  Format.fprintf ppf "'"

let gen_expect_test ppf (`Hex txt as hex) =
  let bin = Hex.to_bytes hex in
  let node = decode_expr bin in
  let whole = Format.asprintf "%a" (pp_node ~wrap:false) node in
  Format.fprintf ppf "# full input: %s@\n" txt;
  Format.fprintf ppf "# full output: %s@\n" whole;
  Format.fprintf ppf "sleep 0.2@\n";
  Format.fprintf ppf "send_async_apdus";
  let apdus = split_sign_apdus bin in
  List.iter
    (fun (apdu, ans) ->
      Format.fprintf ppf "\\@\n  %a %a" Hex.pp (Hex.of_bytes apdu) Hex.pp
        (Hex.of_bytes ans))
    apdus;
  Format.fprintf ppf "@\nsleep 0.5@\n";
  List.iter
    (fun s ->
      Format.fprintf ppf "expect_full_text 'Data' %a\npress_button right@\n"
        shell_escape s)
    (split_screens 38 whole);
  Format.fprintf ppf "press_button both@\nsleep 1@\nexpect_async_apdus_sent@."
