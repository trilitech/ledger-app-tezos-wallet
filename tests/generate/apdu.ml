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

  let of_sk : Tezos_crypto.Signature.secret_key -> t = function
    | Ed25519 _ -> ED25519
    | Secp256k1 _ -> SECP256K1
    | P256 _ -> SECP256R1
    | Bls _ -> failwith "The Bls curve is not supported"

  let to_uint8 = function
    | ED25519 -> 0x00
    | SECP256K1 -> 0x01
    | SECP256R1 -> 0x02
    | BIP32_ED25519 -> 0x03

  let deterministic_sig = function
    | ED25519 -> true
    | SECP256K1 | SECP256R1 | BIP32_ED25519 -> false
end

let success = Bytes.of_string "\x90\x00"
let max_size = 235

let make_packet ?is_last ?(idx = 0) ~cla ~ins ~curve content =
  let content_length = Bytes.length content in
  assert (content_length <= max_size);
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
    if remaining_size <= max_size then
      let packet =
        make_packet ~is_last:true ~idx ~cla ~ins ~curve
        @@ Bytes.sub content ofs remaining_size
      in
      [ packet ]
    else
      let packet =
        make_packet ~idx ~cla ~ins ~curve @@ Bytes.sub content ofs max_size
      in
      packet :: aux (ofs + max_size) (idx + 1)
  in
  aux 0 idx

module Path = struct
  type t = int list

  let to_bytes path =
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
end

module Signer = struct
  type t = {
    path : Path.t;
    pkh : Tezos_crypto.Signature.public_key_hash;
    pk : Tezos_crypto.Signature.public_key;
    sk : Tezos_crypto.Signature.secret_key;
  }

  let make ~path ~sk =
    let open Tezos_crypto.Signature in
    let sk = Secret_key.of_b58check_exn sk in
    let pk = Secret_key.to_public_key sk in
    let pkh = Public_key.hash pk in
    { path; pkh; pk; sk }
end
