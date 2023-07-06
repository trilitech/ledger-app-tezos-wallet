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

let gen_lazy_expr =
  let open QCheck2.Gen in
  let* expr = Gen_micheline.gen_expr in
  return (Protocol.Alpha_context.Script.lazy_expr expr)

let gen_script =
  let open QCheck2.Gen in
  let* code = gen_lazy_expr in
  let* storage = gen_lazy_expr in
  return Protocol.Alpha_context.Script.{ code; storage }

let gen_algo =
  QCheck2.Gen.oneofl Tezos_crypto.Signature.[ Ed25519; Secp256k1; P256 ]

let gen_from_blake ?(tag = Bytes.empty) ?(size = 32) encoding =
  let open QCheck2.Gen in
  let+ bytes = bytes_size (return size) in
  Data_encoding.Binary.of_bytes_exn encoding (Bytes.cat tag bytes)

let some_private_key =
  [
    "edsk3ayZxYsCj8jhgq8ioN4ETy99DqU8yksnq6s7fwcC95ZkDPypkC";
    "edsk41r5Z37fXTTFF5EwQ2Xonr7KsZ8tzvTB3dxCyQn3QsaFbE4MzZ";
    "edsk2w3gCWojHtZdkTFBsWZxvkqEo7zYMPrdmeujyMZsoPfBiFBxsf";
    "edsk3VhsjdEYFg7ZDTL5GB2vnX9Ee9k57GH2VM2emy2Z2A9dcYq8D9";
    "edsk3cWjLMKHahZVFEdvf46VkChebQPd1ctnHWJmLKiui7GHYxxxnK";
    "edsk3GeHYRmSnVAmZZAWQW9wmRWnbJGGGRcEMwkWMpuHH8wKWkuCWv";
    "spsk2vcxPgevMHEU7LTDqg2ypar6gqmwSEz9aM4E3r4mW5B4HMTxDa";
    "spsk39ofnvbpah21bVLzEsEGZDFDZobmh18tde1VGi7CMJNYyFY9Dh";
    "spsk2Ze8YwUjYtmnevdZwQbj7Xyph7orZpTCfeQgyFxCXa3konv9aT";
    "spsk2GiCPV1oR3qnybxGwsHyCcgwFSXL1FVDxZyzDFYTUgrBVJkea5";
    "spsk2xJELkNQ3m7uS9Z6pTtctAmAg76REAME5b8puPvperSofRyU9y";
    "spsk2McHwqWkWm9QmNaHMojwBPUmwvdUKLWmm8XTKoDaW68ayUwE2w";
    "p2sk4APEttm8QwZFsaLq45YaYe3MTWPYwjoGcpvJ6NqRFvUNE8Btmv";
    "p2sk3KhuPgM96N136J2oZ39k4PLG6aUU8QLdMTtgYeShxd1fDtimw7";
    "p2sk3PE1VCKFTutZbfUBf8fwWkutzKhg3DSGgdCNrYLhnxLsPBPc86";
    "p2sk32wjLHQ1iyjnGnH3fPVCiY5MniQnBHBfDosiSeSjPzr7HFLEdM";
    "p2sk3RDMM4PkWDVjEcVTdLru2MifiboZLYrmxwCExUzTnLTuGSygrx";
    "p2sk4BUTMMWuWuWN5yZDYmwgRSDQiMDRT4a1amNwVFWQRWaREmwmxA";
  ]

let gen_secret_key =
  let open QCheck2.Gen in
  let+ sk = oneofl some_private_key in
  Tezos_crypto.Signature.Secret_key.of_b58check_exn sk

let some_public_key =
  [
    "edpkvMUjmJu9CYyKBAjUV3jtU8Y89TemDAcD29bSNh393Bc8z8BH3t";
    "edpkvYT4Cbzg4BFDBMJKEsgUbFsrK8GPtpZcYVUufE88rkSoo7saXo";
    "edpkv5maJHfQAJdLyXefDEVFyK9tfbzKXSKmcT1YYqHc8MmKMphE6v";
    "edpkuGnyR1TBGbWznAvmBfMAFFk321v4bWsHYA5nR75uNyLinK23Go";
    "edpkv1cpS2U1VbviarXs3ZcSdYVXdcad4hVWk3SSGgjFXZUj8xnryJ";
    "edpkuWUfaAWqaxJoG9QKgQRQUHMWfsN1EmoMMXWMwYoE8kjWMWUGDk";
    "sppk7ZT8R42AGSy672NHz9ps6Q4idqWYejAgMwqTWnyYAeq9XZEqWvZ";
    "sppk7a9FzG5f7quhhgiijyiNyktfP9CmqoP6sAa9iV7my2QErYDnk1k";
    "sppk7aVFzHeLSgsqMfKXP3PySiuXXTm7qju9SpqdXFasyNGNu2xiSBB";
    "sppk7busdHfsMPFsp2JYMgY8y3sqyAdbpowhS11BnfZ4RMomy4EQ8V2";
    "sppk7cZHYhQvxTDrsmokcUuJvRTp1amwQdYfGSrPxjL47GRGscDcLkq";
    "sppk7c7M9cBMWAfQM8MPmx9YHpWWVoD8xgdVXPMwBWZb2mtDcFNWMmW";
    "p2pk665znpiyPRWEwpu8tZ7JdNPipkfYpGUhYALjaS4Tm7F7wcx1iRs";
    "p2pk65U84BdxaihUbaJX9qdFzcx7QAJnVioLtvotYbZtehhU6tm1tcg";
    "p2pk64hc2xSWmKEztoTxkRtioDhG46qUfBuhvxpTA69kNEcGsLsPg8T";
    "p2pk66Y3TrkdsL8fYZq6cXBpLqgEVcSZDErYDctJWJc5ZzWkgech8Uj";
    "p2pk68CE92CP8tAjbp6LgNagDcns8acLLv3fPfgkHcBpvyQB4ka81BQ";
    "p2pk66m3NQsd4n6LJWe9WMwx9WHeXwKmBaMwXX92WkMQCR99zmwk2PM";
  ]

let gen_public_key =
  let open QCheck2.Gen in
  let+ pk = oneofl some_public_key in
  Tezos_crypto.Signature.Public_key.of_b58check_exn pk

let some_public_key_hash =
  [
    "tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa";
    "tz1er74kx433vTtpYddGsf3dDt5piBZeeHyQ";
    "tz1McCh72NRhYmJBcWr3zDrLJAxnfR9swcFh";
    "tz1TmFPVZsGQ8MnrBJtnECJgkFUwLa6EWYDm";
    "tz1e8fEumaLvXXe5jV52gejCSt3mGodoKut9";
    "tz1Kp8NCAN5WWwvkWkMmQQXMRe68iURmoQ8w";
    "tz2W3Tvcm64GjcV2bipUynnEsctLFz5Z6yRa";
    "tz2JPgTWZZpxZZLqHMfS69UAy1UHm4Aw5iHu";
    "tz2CJBeWWLsUDjVUDqGZL6od3DeBCNzYXrXk";
    "tz2KC42yW9FXFMJpkUooae2NFYQsM5do3E8H";
    "tz2PPZ2WN4j92Rdx4NM7oW3HAp3x825HUyac";
    "tz2WmivuMG8MMRKMEmzKRMMxMApxZQWYNS4W";
    "tz3XeTwXXJeWNgVR3LqMcyBDdnxjbZ7TeEGH";
    "tz3fLwHKthqhTPK6Lar6CTXN1WbDETw1YpGB";
    "tz3eydffbLkjdVb8zx42BvxpGV87zaRnqL3r";
    "tz3hCsUiQDfneTgD7CSZDaUro8SA5aEhwCp2";
    "tz3Wazpbs4CFj78qv2KBJ8Z7HEyqk6ZPxwWZ";
    "tz3XMQscBFM9vPmpbYMavMmwxRMUWvWGZMQQ";
  ]

let gen_public_key_hash =
  let open QCheck2.Gen in
  let pick =
    let+ pkh = oneofl some_public_key_hash in
    Tezos_crypto.Signature.Public_key_hash.of_b58check_exn pkh
  in
  let gen =
    let ed25519_tag = Bytes.of_string "\000" in
    let secp256k1_tag = Bytes.of_string "\001" in
    let p256_tag = Bytes.of_string "\002" in
    let public_key_hash_size = 20 in
    let* tag = oneofl [ ed25519_tag; secp256k1_tag; p256_tag ] in
    gen_from_blake ~tag ~size:public_key_hash_size
      Tezos_crypto.Signature.Public_key_hash.encoding
  in
  oneof [ pick; gen ]

let some_sc_rollup_hash =
  [
    "sr18hRM2ke5FCVvjhqkDAhGPMzbPCrzJ8wrU";
    "sr1KCJxqn1tZAXqHXVszTtf6F3QnffhjQqCh";
    "sr1EonJTemPhrmLNCFC6XT8gx5fNsXSvKQ2F";
    "sr1B7oLgQietkTsrfwDVicYv8G7cQ3L6scZa";
    "sr1Lcs3wS4mDi3UsFV9ULnhofE81FN2JmPvB";
    "sr1MyCwR83hZphCSqaYSQApPxPMeyksJWWnh";
    "sr1Sg4yX2RfUoBqGjc8aFwB5cG5Lt5t7Guws";
    "sr1TPCJhiXQJ2VhCZEbCG1kdzqPEPTsfEEdr";
    "sr1HMSLDEvsoq8LZb3cD7f3ayhi7JVVvWfpZ";
    "sr1FSK2QXZjfsp8S9U3EhWmn8Bxp2SdE7Khf";
  ]

let gen_sc_rollup_hash =
  let open QCheck2.Gen in
  let+ sc_h = oneofl some_sc_rollup_hash in
  Protocol.Alpha_context.Sc_rollup_repr.Address.of_b58check_exn sc_h

let some_sc_rollup_commiment_hash =
  [
    "src12UJzB8mg7yU6nWPzicH7ofJbFjyJEbHvwtZdfRXi8DQHNp1LY8";
    "src149FtYiXSAwm1PBBavezYQ2UEg1YbuBzt5KwVE54hx2tbnPtzH2";
    "src14Cw34Jvg6v9efy46QaYBqfVxBruvCYSHBu8fVyVTx4P5f4b3o6";
    "src12cn216Y5KX2dxLRq4Hsmb1QQ9zVZUN4yd1yzPY5oJjS8kHUnNJ";
    "src13cokfHxjwy31qfKwWk1KE6zbpLA7vG6T1wMwLVQMuVaYyBkijH";
    "src143oifrHPmYvdBYHyZDEnUJMZJzLxWhkxC8vAoxM4qdyTsADhbc";
    "src12iKxUYgPaHzFPwTNasYNLsd5k6atiLBWgsec3aBNRL8WttwuXP";
    "src13nsaWXcAhKPJ1UftmHk1xhp6YTpLJh7Wuf2mPFxgAbBLMAyE7H";
    "src13N7L61eHYEW116jhA1U7yJpeaAuMQQx18f6LfdY6DG2k347HV3";
    "src13Bexybvi9ps398rBiKgaJy3nRY6xyE111eZx5vZ2epNZcdxLtX";
    "src13J76oinSMSPoabJJ9DXArwPaZfhDn9AYzBQMUDd3NynyDSuN1r";
  ]

let gen_sc_rollup_commiment_hash =
  let open QCheck2.Gen in
  let+ sc_ch = oneofl some_sc_rollup_commiment_hash in
  Protocol.Alpha_context.Sc_rollup.Commitment.Hash.of_b58check_exn sc_ch

let gen_tez =
  QCheck2.Gen.map Protocol.Alpha_context.Tez.(mul_exn one_cent) QCheck2.Gen.nat

let gen_manager_counter =
  QCheck2.Gen.map
    Protocol.Alpha_context.Manager_counter.Internal_for_tests.of_int
    QCheck2.Gen.nat

let gen_z_bound = QCheck2.Gen.map Z.of_int QCheck2.Gen.nat

let gen_gaz_bound =
  QCheck2.Gen.map Protocol.Alpha_context.Gas.Arith.integral_of_int_exn
    QCheck2.Gen.nat

let gen_ticket_amount =
  let open QCheck2.Gen in
  let+ strict_nat = pint ~origin:1 in
  let ticket_amount = Protocol.Ticket_amount.of_zint (Z.of_int strict_nat) in
  Option.get ticket_amount

let gen_origination_nonce =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let rec incr i nonce =
    if i <= 0 then return nonce
    else
      let nonce = Origination_nonce.Internal_for_tests.incr nonce in
      incr (i - 1) nonce
  in
  let* i = small_nat in
  let initial_nonce =
    Origination_nonce.Internal_for_tests.initial Environment.Operation_hash.zero
  in
  incr i initial_nonce

let some_contract_hash =
  [
    "KT1BEqzn5Wx8uJrZNvuS9DVHmLvG9td3fDLi";
    "KT1Vk1K7SWCZKhaF5KiuP7GEpPZ6wzukaUpG";
    "KT1TKXZqQa2D5ZC2VgYdCtoyPReUSCasmJ61";
    "KT1QWdbASvaTXW8GWfhfNh3JMjgXvnZAATJW";
    "KT1SnUMh6K7M2Rdz1AvLtxZSmeErhJUr3wH9";
    "KT1VogMpwUD8xJR7pJMwhbCnTkJGM92WD2NL";
    "KT1Bt8FwiW7EEQyfB3iLz7iaJ3eSYvrYoTvv";
    "KT1TGa4r6j84k8vFc9QsfAJmTQfVtwGmh6ss";
    "KT1GqFTwDrEnowTZaHr5dEFo5CLX75fAXCTW";
    "KT1QWdbASvaTXW8GWfhfNh3JMjgXvnZAATJW";
    "KT1HMCxCyeGbZaGBsLMKVyMbMRzFpZBxKoY7";
    "KT1XULaVx5dQmQPuWsNcZod3Dk2N9ZFQmfQp";
    "KT1VvcxT3QHBjJd2EKkREXyYQBi7LVcCRc5H";
    "KT1DfDzRjbXnTGogM2Nih8mpXcSVM54igtNs";
    "KT1Mjjcb6tmSsLm7Cb3DSQszePjfchPM4Uxm";
    "KT1GW4QHn66m7WWWMWCMNaWmGYpCRbg5ahwU";
  ]

let gen_contract_hash =
  let open QCheck2.Gen in
  let pick =
    let+ contract = oneofl some_contract_hash in
    Protocol.Contract_hash.of_b58check_exn contract
  in
  let gen =
    gen_from_blake ~size:Protocol.Contract_hash.size
      Protocol.Contract_hash.encoding
  in
  oneof [ pick; gen ]

let gen_contract =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let gen_implict =
    let* public_key_hash = gen_public_key_hash in
    return (Contract.Implicit public_key_hash)
  in
  let gen_originated =
    let* contract_hash = gen_contract_hash in
    return (Contract.Originated contract_hash)
  in
  oneof [ gen_implict; gen_originated ]

let gen_entrypoint =
  QCheck2.Gen.oneofl
    Protocol.Alpha_context.Entrypoint.
      [
        default;
        root;
        do_;
        set_delegate;
        remove_delegate;
        deposit;
        of_string_strict_exn "jean_bob";
      ]

let gen_delegation =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let* public_key_hash_opt = option gen_public_key_hash in
  return (Delegation public_key_hash_opt)

let gen_increase_paid_storage =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let* amount_in_bytes = gen_z_bound in
  let* destination = gen_contract_hash in
  return (Increase_paid_storage { amount_in_bytes; destination })

let gen_origination =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let* delegate = option gen_public_key_hash in
  let* script = gen_script in
  let* credit = gen_tez in
  return (Origination { delegate; script; credit })

let gen_register_global_constant =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let* value = gen_lazy_expr in
  return (Register_global_constant { value })

let gen_reveal =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let* public_key = gen_public_key in
  return (Reveal public_key)

let gen_set_deposits_limit =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let* tez_opt = QCheck2.Gen.option gen_tez in
  return (Set_deposits_limit tez_opt)

let gen_transaction =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let* amount = gen_tez in
  let* destination = gen_contract in
  let* entrypoint = gen_entrypoint in
  let* parameters = gen_lazy_expr in
  return (Transaction { amount; destination; entrypoint; parameters })

let gen_transfer_ticket =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let* contents = gen_lazy_expr in
  let* ty = gen_lazy_expr in
  let* ticketer = gen_contract in
  let* amount = gen_ticket_amount in
  let* destination = gen_contract in
  let* entrypoint = gen_entrypoint in
  return
    (Transfer_ticket { contents; ty; ticketer; amount; destination; entrypoint })

let gen_update_consensus_key =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let* public_key = gen_public_key in
  return (Update_consensus_key public_key)

let gen_sc_rollup_add_messages =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let* messages = list_size small_nat (string_size small_nat) in
  return (Sc_rollup_add_messages { messages })

let gen_sc_rollup_execute_outbox_message =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let* rollup = gen_sc_rollup_hash in
  let* cemented_commitment = gen_sc_rollup_commiment_hash in
  let* output_proof = string_size nat in
  return
    (Sc_rollup_execute_outbox_message
       { rollup; cemented_commitment; output_proof })

let gen_failing_noop =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let* message = string_size nat in
  return (Failing_noop message)

let gen_manager_operation gen_operation =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let* source = gen_public_key_hash in
  let* fee = gen_tez in
  let* counter = gen_manager_counter in
  let* gas_limit = gen_gaz_bound in
  let* storage_limit = gen_z_bound in
  let* operation = gen_operation in
  return
    (Manager_operation
       { source; fee; counter; operation; gas_limit; storage_limit })

type hidden_operation =
  | HO : _ Protocol.Alpha_context.contents -> hidden_operation

let gen_hidden_operation =
  let aux gen_operation =
    QCheck2.Gen.map (fun operation -> HO operation) gen_operation
  in
  [ aux gen_failing_noop ]

type hidden_manager_operation =
  | HMO :
      _ Protocol.Alpha_context.Kind.manager Protocol.Alpha_context.contents
      -> hidden_manager_operation

let gen_hidden_manager_operation =
  let aux gen_operation =
    QCheck2.Gen.map
      (fun manager_operation -> HMO manager_operation)
      (gen_manager_operation gen_operation)
  in
  [
    aux gen_delegation;
    aux gen_increase_paid_storage;
    aux gen_origination;
    aux gen_register_global_constant;
    aux gen_reveal;
    aux gen_set_deposits_limit;
    aux gen_transaction;
    aux gen_transfer_ticket;
    aux gen_update_consensus_key;
    aux gen_sc_rollup_add_messages;
    aux gen_sc_rollup_execute_outbox_message;
  ]

type hidden_manager_operation_list =
  | HMOL :
      _ Protocol.Alpha_context.Kind.manager Protocol.Alpha_context.contents_list
      -> hidden_manager_operation_list

let gen_packed_contents_list =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let gen_packed_manager_operation_list =
    (* Do not make operation batches too large *)
    let* size = int_range 1 3 in
    let rec aux i =
      if i <= 1 then
        let* (HMO manager_operation) = oneof gen_hidden_manager_operation in
        return (HMOL (Single manager_operation))
      else
        let* (HMO manager_operation) = oneof gen_hidden_manager_operation in
        let* (HMOL contents_list) = aux (i - 1) in
        return (HMOL (Cons (manager_operation, contents_list)))
    in
    let* (HMOL contents_list) = aux size in
    return (Contents_list contents_list)
  in
  let gen_single_operation =
    let+ (HO operation) = oneof gen_hidden_operation in
    Contents_list (Single operation)
  in
  frequency
    [
      ( List.length gen_hidden_manager_operation,
        gen_packed_manager_operation_list );
      (List.length gen_hidden_operation, gen_single_operation);
    ]

let gen () =
  let shell =
    { Tezos_base.Operation.branch = Tezos_crypto.Hashed.Block_hash.zero }
  in
  let contents =
    QCheck2.Gen.generate1 ~rand:Gen_utils.random_state gen_packed_contents_list
  in
  (shell, contents)

let op = Seq.forever gen
let watermark = Tezos_crypto.Signature.Generic_operation

let encode op =
  Data_encoding.Binary.to_bytes_exn
    Protocol.Alpha_context.Operation.unsigned_encoding op

let decode bin =
  Data_encoding.Binary.of_bytes_exn
    Protocol.Alpha_context.Operation.unsigned_encoding bin

let hex =
  Seq.map
    (fun op ->
      let bin = encode op in
      let hex = Hex.of_bytes bin in
      (op, bin, hex))
    op
