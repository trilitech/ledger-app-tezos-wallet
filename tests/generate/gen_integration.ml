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

let start_speculos ppf mnemonic =
  Format.fprintf ppf "start_speculos \"%a\"@." Tezos_client_base.Bip39.pp
    mnemonic

let expect_full_text ppf l =
  let pp_space ppf () = Format.fprintf ppf " " in
  let pp_args = Format.pp_print_list ~pp_sep:pp_space shell_escape in
  Format.fprintf ppf "expect_full_text %a@." pp_args l

let expect_section_content ppf ~title content =
  Format.fprintf ppf "expect_section_content %a %a@." shell_escape title
    shell_escape content

let expected_review_operation ppf () =
  Format.fprintf ppf "expected_review_operation@."

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
  Format.fprintf ppf "expect_apdu_return %a" pp_hex_bytes ans

type async_apdu = { packet : bytes; check : Format.formatter -> unit -> unit }

let send_async_apdus ppf async_apdus =
  let pp_dash_break_line ppf () = Format.fprintf ppf " \\@\n\t" in
  let pp_async_apdu ppf { packet; check } =
    Format.fprintf ppf "%a \"%a\"" pp_hex_bytes packet check ()
  in
  let pp_async_apdus =
    Format.pp_print_list ~pp_sep:pp_dash_break_line pp_async_apdu
  in
  Format.fprintf ppf "send_async_apdus%a%a@." pp_dash_break_line ()
    pp_async_apdus async_apdus

let expect_async_apdus_sent ppf () =
  Format.fprintf ppf "expect_async_apdus_sent@."

let expected_home ppf () = Format.fprintf ppf "expected_home@."
let expect_exited ppf () = Format.fprintf ppf "expect_exited@."
let quit_app ppf () = Format.fprintf ppf "quit_app@."

let check_tlv_signature ppf ~prefix ~suffix pk message =
  Format.fprintf ppf "check_tlv_signature %a %a %a %a" pp_hex_bytes prefix
    pp_hex_bytes suffix Tezos_crypto.Signature.Public_key.pp pk pp_hex_bytes
    message

(** Specific *)

let expect_accept ppf () = Format.fprintf ppf "expected_accept@."
let expect_reject ppf () = Format.fprintf ppf "expected_reject@."

let accept ppf () =
  expect_accept ppf ();
  Button.(press ppf Both)

let reject ppf () =
  expect_reject ppf ();
  Button.(press ppf Both)

let set_expert_mode ppf () = Format.fprintf ppf "set_expert_mode@."

type screen = { title : string; contents : string }

let make_screen ~title contents = { title; contents }

let expected_screen ppf { title; contents } =
  expect_section_content ppf ~title contents

let go_through_screens ppf screens =
  List.iter
    (fun screen ->
      expected_screen ppf screen;
      Button.(press ppf Right))
    screens

let need_expert_mode_screen ?(expert_mode = true) title =
  if expert_mode then
    { title = "Next field requires"; contents = "careful review" }
  else { title; contents = "Needs Expert mode" }

let sign ppf ~signer:Apdu.Signer.({ sk; pk; _ } as signer) ~watermark bin =
  let watermark = Tezos_crypto.Signature.bytes_of_watermark watermark in
  let bin = Bytes.cat watermark bin in
  let packets = Apdu.sign ~signer bin in
  let bin_accept_check ppf () =
    let bin_hash = Tezos_crypto.Blake2B.(to_bytes (hash_bytes [ bin ])) in
    if Apdu.Curve.(deterministic_sig (of_sk sk)) then
      let sign =
        Tezos_crypto.Signature.to_bytes (Tezos_crypto.Signature.sign sk bin)
      in
      expect_apdu_return ppf
        (Bytes.concat Bytes.empty [ bin_hash; sign; Apdu.success ])
    else check_tlv_signature ppf ~prefix:bin_hash ~suffix:Apdu.success pk bin
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

open Tezos_protocol_018_Proxford
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

let node_to_screens ~title ppf node =
  let whole = Format.asprintf "%a" (pp_node ~wrap:false) node in
  Format.fprintf ppf "# full output: %s@\n" whole;
  [ make_screen ~title whole ]

let pp_opt_field pp ppf = function
  | None -> Format.fprintf ppf "Field unset"
  | Some v -> Format.fprintf ppf "%a" pp v

let pp_tz ppf tz = Format.fprintf ppf "%a XTZ" Protocol.Alpha_context.Tez.pp tz

let pp_lazy_expr ppf lazy_expr =
  let expr = Result.get_ok @@ Protocol.Script_repr.force_decode lazy_expr in
  Format.fprintf ppf "%a" (pp_node ~wrap:false) (Micheline.root expr)

let pp_string_binary ppf s = Format.fprintf ppf "%a" Hex.pp (Hex.of_string s)

let pp_serialized_proof ppf proof =
  let proof =
    Data_encoding.(
      proof
      |> Binary.to_bytes_exn
           Protocol.Alpha_context.Sc_rollup.Proof.serialized_encoding
      |> Binary.of_bytes_exn (string' Hex))
  in
  pp_string_binary ppf proof

let operation_to_screens
    ( (_shell : Tezos_base.Operation.shell_header),
      (Contents_list contents : Protocol.Alpha_context.packed_contents_list) ) =
  let open Protocol.Alpha_context in
  let make_screen ~title fmt = Format.kasprintf (make_screen ~title) fmt in
  let make_screens ~title pp values =
    let aux i value =
      let title = Format.sprintf "%s (%d)" title i in
      make_screen ~title "%a" pp value
    in
    List.mapi aux values
  in
  let first_expert_mode_screen =
    let is_first = ref true in
    fun title ->
      if !is_first then (
        is_first := false;
        [ need_expert_mode_screen title ])
      else []
  in
  let screen_of_manager n (type t)
      (Manager_operation { source; fee; operation; storage_limit; _ } :
        t Kind.manager contents) =
    let aux ~kind operation_screens =
      let operation_index = Format.asprintf "Operation (%d)" n in
      let manager_screens =
        [
          make_screen ~title:operation_index "%s" kind;
          make_screen ~title:"Source" "%a"
            Tezos_crypto.Signature.Public_key_hash.pp source;
          make_screen ~title:"Fee" "%a" pp_tz fee;
          make_screen ~title:"Storage limit" "%s" (Z.to_string storage_limit);
        ]
      in
      manager_screens @ operation_screens
    in
    match operation with
    | Delegation public_key_hash_opt ->
        aux ~kind:"Delegation"
          [
            make_screen ~title:"Delegate" "%a"
              (pp_opt_field Tezos_crypto.Signature.Public_key_hash.pp)
              public_key_hash_opt;
          ]
    | Increase_paid_storage { amount_in_bytes; destination } ->
        aux ~kind:"Increase paid storage"
          [
            make_screen ~title:"Amount" "%s" (Z.to_string amount_in_bytes);
            make_screen ~title:"Destination" "%a" Protocol.Contract_hash.pp
              destination;
          ]
    | Origination { delegate; script = { code; storage }; credit } ->
        aux ~kind:"Origination"
        @@ [
             make_screen ~title:"Balance" "%a" pp_tz credit;
             make_screen ~title:"Delegate" "%a"
               (pp_opt_field Tezos_crypto.Signature.Public_key_hash.pp)
               delegate;
           ]
        @ first_expert_mode_screen "Code"
        @ [
            make_screen ~title:"Code" "%a" pp_lazy_expr code;
            make_screen ~title:"Storage" "%a" pp_lazy_expr storage;
          ]
    | Register_global_constant { value } ->
        aux ~kind:"Register global constant"
        @@ first_expert_mode_screen "Value"
        @ [ make_screen ~title:"Value" "%a" pp_lazy_expr value ]
    | Reveal public_key ->
        aux ~kind:"Reveal"
          [
            make_screen ~title:"Public key" "%a"
              Tezos_crypto.Signature.Public_key.pp public_key;
          ]
    | Set_deposits_limit tez_opt ->
        aux ~kind:"Set deposit limit"
          [
            make_screen ~title:"Staking limit" "%a" (pp_opt_field pp_tz) tez_opt;
          ]
    | Transaction { amount; entrypoint; destination; parameters } ->
        let parameter =
          if
            Protocol.Script_repr.is_unit_parameter parameters
            && Protocol.Entrypoint_repr.is_default entrypoint
          then []
          else
            [ make_screen ~title:"Entrypoint" "%a" Entrypoint.pp entrypoint ]
            @ first_expert_mode_screen "Parameter"
            @ [ make_screen ~title:"Parameter" "%a" pp_lazy_expr parameters ]
        in
        aux ~kind:"Transaction"
          ([
             make_screen ~title:"Amount" "%a" pp_tz amount;
             make_screen ~title:"Destination" "%a" Contract.pp destination;
           ]
          @ parameter)
    | Transfer_ticket
        { contents; ty; ticketer; amount; destination; entrypoint } ->
        aux ~kind:"Transfer ticket"
        @@ first_expert_mode_screen "Contents"
        @ [
            make_screen ~title:"Contents" "%a" pp_lazy_expr contents;
            make_screen ~title:"Type" "%a" pp_lazy_expr ty;
            make_screen ~title:"Ticketer" "%a" Contract.pp ticketer;
            make_screen ~title:"Amount" "%s"
              Protocol.Script_int.(to_string (amount :> n num));
            make_screen ~title:"Destination" "%a" Contract.pp destination;
            make_screen ~title:"Entrypoint" "%a" Entrypoint.pp entrypoint;
          ]
    | Update_consensus_key public_key ->
        aux ~kind:"Set consensus key"
          [
            make_screen ~title:"Public key" "%a"
              Tezos_crypto.Signature.Public_key.pp public_key;
          ]
    | Sc_rollup_add_messages { messages } ->
        aux ~kind:"SR: send messages"
        @@ make_screens ~title:"Message" pp_string_binary messages
    | Sc_rollup_execute_outbox_message
        { rollup; cemented_commitment; output_proof } ->
        aux ~kind:"SR: execute outbox message"
        @@ [
             make_screen ~title:"Rollup" "%a" Sc_rollup.Address.pp rollup;
             make_screen ~title:"Commitment" "%a" Sc_rollup.Commitment.Hash.pp
               cemented_commitment;
           ]
        @ first_expert_mode_screen "Output proof"
        @ [
            make_screen ~title:"Output proof" "%a" pp_string_binary output_proof;
          ]
    | Sc_rollup_originate { kind; boot_sector; parameters_ty; whitelist } ->
        let whitelist =
          match whitelist with
          | None | Some [] -> []
          | Some whitelist ->
              make_screens ~title:"Whitelist"
                Tezos_crypto.Signature.Public_key_hash.pp whitelist
        in
        aux ~kind:"SR: originate"
        @@ [ make_screen ~title:"Kind" "%a" Sc_rollup.Kind.pp kind ]
        @ first_expert_mode_screen "Kernel"
        @ [
            make_screen ~title:"Kernel" "%a" pp_string_binary boot_sector;
            make_screen ~title:"Parameters" "%a" pp_lazy_expr parameters_ty;
          ]
        @ whitelist
    | _ -> assert false
  in
  let screen_of_operation (type t) (operation : t contents) =
    let aux ~kind operation_screens =
      make_screen ~title:"Operation (0)" "%s" kind :: operation_screens
    in
    match operation with
    | Failing_noop message ->
        aux ~kind:"Failing noop"
          [ make_screen ~title:"Message" "%a" pp_string_binary message ]
    | Proposals { source; period; proposals } ->
        aux ~kind:"Proposals"
          ([
             make_screen ~title:"Source" "%a"
               Tezos_crypto.Signature.Public_key_hash.pp source;
             make_screen ~title:"Period" "%ld" period;
           ]
          @ make_screens ~title:"Proposal" Tezos_crypto.Hashed.Protocol_hash.pp
              proposals)
    | Ballot { source; period; proposal; ballot } ->
        aux ~kind:"Ballot"
          [
            make_screen ~title:"Source" "%a"
              Tezos_crypto.Signature.Public_key_hash.pp source;
            make_screen ~title:"Period" "%ld" period;
            make_screen ~title:"Proposal" "%a"
              Tezos_crypto.Hashed.Protocol_hash.pp proposal;
            make_screen ~title:"Ballot" "%a" Vote.pp_ballot ballot;
          ]
    | Manager_operation _ | _ -> assert false
  in
  let rec screen_of_operations : type t. int -> t contents_list -> screen list =
   fun n -> function
    | Single (Manager_operation _ as m) -> screen_of_manager n m
    | Cons ((Manager_operation _ as m), rest) ->
        let screen_of_manager = screen_of_manager n m in
        let screen_of_operations = screen_of_operations (succ n) rest in
        screen_of_manager @ screen_of_operations
    | Single op -> screen_of_operation op
  in
  screen_of_operations 0 contents

let mnemonic_of_string s =
  Option.get @@ Tezos_client_base.Bip39.of_words @@ String.split_on_char ' ' s

let zebra =
  mnemonic_of_string
    "zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra \
     zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra"

let seed12 =
  mnemonic_of_string
    "spread husband dish drop file swap dog foil verb nut orient chicken"

let seed15 =
  mnemonic_of_string
    "tonight jump steak foil time sudden detect text rifle liberty volcano \
     riot person guess unaware"

let seed21 =
  mnemonic_of_string
    "around dignity equal spread between young lawsuit interest climb wide \
     that panther rather mom snake scene ecology reunion ice illegal brush"

let seed24 =
  mnemonic_of_string
    "small cool word clock copy badge jungle pole cool horror woman miracle \
     silver library foster cinnamon spring discover gauge faint hammer test \
     air cream"

let default_path = [ 44; 1729; 0; 0 ]
let path_0 = [ 0 ]
let path_2_11_5 = [ 2; 11; 5 ]
let path_17_8_6_9 = [ 17; 8; 6; 9 ]
let path_9_12_13_8_78 = [ 9; 12; 13; 8; 78 ]

let tz1_signers =
  [
    (* tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E *)
    Apdu.Signer.make ~mnemonic:zebra ~path:default_path
      ~sk:"edsk2tUyhVvGj9B1S956ZzmaU4bC9J7t8xVBH52fkAoZL25MHEwacd";
    (* tz1NxVR43U25oudr9QPCjXycDNaH1arZZ5fL *)
    Apdu.Signer.make ~mnemonic:seed12 ~path:path_0
      ~sk:"edsk3xdyqj4YH64DKQ3ihVktjs2x5DagcoP3yNbdqPg3FAibHM1EU2";
    (* tz1PVqTSCw2JKxXcwSLgd5Zad18djpvcvPMF *)
    Apdu.Signer.make ~mnemonic:seed15 ~path:path_2_11_5
      ~sk:"edsk3qtimAD78T5PBdkAXwjPHYa1tEXJ3jmMcVgNSXuXs9wjsnWnrX";
    (* tz1aex9GimxigprKi8MQK3j6sSisDpbtRBXw *)
    Apdu.Signer.make ~mnemonic:seed21 ~path:path_17_8_6_9
      ~sk:"edsk3xW7FFGdpFTUzqd2dbRVDGt6vK2XrRfQU5VjXKFABdN3mbmRkH";
    (* tz1ZAxU3Q39BSJWoHzEcPmt8U36iHmdCcZ2K *)
    Apdu.Signer.make ~mnemonic:seed24 ~path:path_9_12_13_8_78
      ~sk:"edsk2vPx2hnTsef6EeuD846PoHTja7dM96YtZ26S3Xz4KP36a2m6k1";
    (* tz1NKmt8bGYc5guuJyXPTkEbvBHXHBHVALhc *)
    Apdu.Signer.make ~mnemonic:seed12 ~path:default_path
      ~sk:"edsk3q2GyniEU2jGxw7uLFPys4AfALs96VE64d7kr7NXhtN4w8RUue";
    (* tz1aKnxJdtJQ89Hp5HeXMxgJnnyNHjyAUmSb *)
    Apdu.Signer.make ~mnemonic:seed15 ~path:path_0
      ~sk:"edsk35nft3nYCBpzdXqKCL1VPpVTa3NFTDGj7QqSxc1auzkjz2znTW";
    (* tz1VeeXEdqU9TjfAvPNKpUopB1jXJAZh9egp *)
    Apdu.Signer.make ~mnemonic:seed21 ~path:path_2_11_5
      ~sk:"edsk3VTt2ikBiYrt1LKM9juE71Tfg6YmjdUGR4PzT2wuV5uEJ9vTDM";
    (* tz1dHwBzesqaqdNdsMGDu2xjAPvirzmx4BTL *)
    Apdu.Signer.make ~mnemonic:seed24 ~path:path_17_8_6_9
      ~sk:"edsk3mfq95CqPnQgcj38KpAz8taNjQY5tQYvNRKojXLKKjhZC72z2H";
    (* tz1ez9eEyAxuDZACjHdCXym43UQDdMNa3LEL *)
    Apdu.Signer.make ~mnemonic:zebra ~path:path_9_12_13_8_78
      ~sk:"edsk3eZBgFAf1VtdibfxoCcihxXje9S3th7jdEgVA2kHG82EKYNKNm";
  ]

let tz2_signers =
  [
    (* tz2GB5YHqF4UzQ8GP5yUqdhY9oVWRXCY2hPU *)
    Apdu.Signer.make ~mnemonic:zebra ~path:default_path
      ~sk:"spsk2Pfx9chqXVbz2tW7ze4gGU4RfaiK3nSva77bp69zHhFho2zTze";
    (* tz2ABzm53J2rYC2JH9tHcKdHHupHvmzbpKBs *)
    Apdu.Signer.make ~mnemonic:seed12 ~path:path_0
      ~sk:"spsk3AexEMWMxpMTuYCu9X4CXuksjhbzg1CDGSromY8Vi4tWJp2cyX";
    (* tz2E5kDU7kYqDVm3tDYWE5Hn1NadMbGgjGP1 *)
    Apdu.Signer.make ~mnemonic:seed15 ~path:path_2_11_5
      ~sk:"spsk1nQZNy2MmgEN1RJ2TYvWHb217wep34hKLHXmRWZ8acnydhVz1u";
    (* tz2SDC3uomE2dYkWrjywGttCXqbgRPnKiTbc *)
    Apdu.Signer.make ~mnemonic:seed21 ~path:path_17_8_6_9
      ~sk:"spsk1i8iYLWU3hJqXrQ15PNnTi3jHkQePDPatWyCpViC1hqfVRcn4t";
    (* tz2JmCpSXB3UfcvJ81MvfKE5wnnrwiDthkoK *)
    Apdu.Signer.make ~mnemonic:seed24 ~path:path_9_12_13_8_78
      ~sk:"spsk2Z8YhgcbTRy9DTFqjjgzjKF9GZja6L66nL7rZzN5z4THMXC1Ma";
    (* tz2Ps3rRYoE9NQALtq3GP3q2xa4PGCyyEeYc *)
    Apdu.Signer.make ~mnemonic:seed12 ~path:default_path
      ~sk:"spsk1cWQyarGSriaDPeoFYMh4N5V2MVxZiM5cxUzWMW5FfqCCvyy4z";
    (* tz2BuPTGejxZFTiHmqxZn2iY38W1Qt9FA73o *)
    Apdu.Signer.make ~mnemonic:seed15 ~path:path_0
      ~sk:"spsk2RCfKCD8Wq9P37LTotWD1F1oCvFFSzUgMSm5aEyaWndyy7gDys";
    (* tz2RABsKSVgw5fgAJobpvZV1QLrtwBFBZzfb *)
    Apdu.Signer.make ~mnemonic:seed21 ~path:path_2_11_5
      ~sk:"spsk2DMEFbbzny6LjSXoqjJMgu1QQmHjPjb3QQcyYyY4AWjUDEvV5p";
    (* tz2HTV4fDogX6gqrdv64y6iRQb16gxqdQetR *)
    Apdu.Signer.make ~mnemonic:seed24 ~path:path_17_8_6_9
      ~sk:"spsk2nkrtUc4aYtErsvfoZcuj96HCV2KXtKfk6dbJLRZG6MQQnUEjp";
    (* tz2XUkdzf3jbAxsDFd4N2iWS5BtB5UfAZf73 *)
    Apdu.Signer.make ~mnemonic:zebra ~path:path_9_12_13_8_78
      ~sk:"spsk2TDhUqfr8HV5643ubsEod5B8iWH7dAEE46mimatSQPs9uJaUsW";
  ]

let tz3_signers =
  [
    (* tz3UMNyvQeMj6mQSftW2aV2XaWd3afTAM1d5 *)
    Apdu.Signer.make ~mnemonic:zebra ~path:default_path
      ~sk:"p2sk2zPCmKo6zTSjPbDHnLiHtPAqVRFrExN3oTvKGbu3C99Jyeyura";
    (* tz3UCiS9hCP5Z4H4NY1Sh8MQFKGAR2dBwt8X *)
    Apdu.Signer.make ~mnemonic:seed12 ~path:path_0
      ~sk:"p2sk33tVU1LvLBLaPcvT35fedM5p2cqncRMntkjggXBNbzW78zd119";
    (* tz3dXAtT2DC9uxJz3Kzd73aAf5u8Duk1U2L4 *)
    Apdu.Signer.make ~mnemonic:seed15 ~path:path_2_11_5
      ~sk:"p2sk2eUahqgSwd7Dd7NgnRgDSpZjbBNkUZF16vNuDGTQvYvznWiWkB";
    (* tz3Mo3LbcwtMZcNELoVcqMQcE6Lkt2wh1N6a *)
    Apdu.Signer.make ~mnemonic:seed21 ~path:path_17_8_6_9
      ~sk:"p2sk47tCEh6jyBwzXbunTVik9cr1rBHQ1bK6tFUztdyY8UNM1sFMuU";
    (* tz3NU86vCJ9hYLVRnrQybCFrGpAxJeFgKNSb *)
    Apdu.Signer.make ~mnemonic:seed24 ~path:path_9_12_13_8_78
      ~sk:"p2sk4AwmdZy89ZGb4qfrSZeDxfTP75b7q8gsWBBuYMQf7XqtxS7V6H";
    (* tz3YHnyWXEePpJSN4Wo9zTGrujVVJFvhSMoW *)
    Apdu.Signer.make ~mnemonic:seed12 ~path:default_path
      ~sk:"p2sk4ASENf9NtKzvNV9psrM5sLCb3SduXpgF4Gw92pX3EeKWNszcQ9";
    (* tz3RGUhYMGXgguVwNVV6mjhyuAx8z1ZwbKLw *)
    Apdu.Signer.make ~mnemonic:seed15 ~path:path_0
      ~sk:"p2sk2uQBEEebh9Y1PGJASM1qauCeTMDLZxuLmYzK7qVwRAHUiZNyet";
    (* tz3MLzYAp97KJAyBnDqfQmB5YE9VX1HbcX5v *)
    Apdu.Signer.make ~mnemonic:seed21 ~path:path_2_11_5
      ~sk:"p2sk3a2kULUSRZuBhDJwNW4uzQxNm9BmuTYPd72CNrEPkhiXtM8eVn";
    (* tz3eddqo9pvgVGirAoahe8swa9tJL5GQeAix *)
    Apdu.Signer.make ~mnemonic:seed24 ~path:path_17_8_6_9
      ~sk:"p2sk4ByXhrxeFLPkgusLA1VSu19WhQqbuAd23Ynq7k8bb7TqyezRrS";
    (* tz3XZs3igEN9SQcEXKcroC9GgA6JR4SAR1FK *)
    Apdu.Signer.make ~mnemonic:zebra ~path:path_9_12_13_8_78
      ~sk:"p2sk37UGsn84Lkbamg5JsKfmNXjkAcqt8Qic6eFTkGoVccP6KRPzse";
  ]

let gen_signer = QCheck2.Gen.oneofl (tz1_signers @ tz2_signers @ tz3_signers)

let gen_expect_test_sign ?(expert_mode = false) ppf ~watermark bin screens =
  Format.fprintf ppf "# full input: %a@." pp_hex_bytes bin;
  let signer = QCheck2.Gen.generate1 ~rand:Gen_utils.random_state gen_signer in
  Format.fprintf ppf "# signer: %a@." Tezos_crypto.Signature.Public_key_hash.pp
    signer.pkh;
  Format.fprintf ppf "# path: %a@." Apdu.Path.pp signer.path;
  start_speculos ppf signer.mnemonic;
  expected_home ppf ();
  if expert_mode then set_expert_mode ppf ();
  sign ppf ~signer ~watermark bin;
  expected_review_operation ppf ();
  Button.(press ppf Right);
  go_through_screens ppf screens;
  accept ppf ();
  expect_async_apdus_sent ppf ();
  quit_app ppf ()

let gen_expect_test_sign_micheline_data ppf bin =
  let screens =
    let node = Gen_micheline.decode bin in
    node_to_screens ~title:"Expression" ppf (Micheline.root node)
  in
  gen_expect_test_sign ppf ~watermark:Gen_micheline.watermark bin screens

let gen_expect_test_sign_operation ppf bin =
  let screens =
    let op = Gen_operations.decode bin in
    operation_to_screens op
  in
  gen_expect_test_sign ~expert_mode:true ppf ~watermark:Gen_operations.watermark
    bin screens
