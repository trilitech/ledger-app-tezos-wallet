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

open Tezos_protocol_016_PtMumbai
open Tezos_benchmarks_proto_016_PtMumbai

(* Only use this function to generate values *)
let gen_expr =
  let gen rand =
    Protocol.Script_repr.lazy_expr
      (Michelson_generation.make_code_sampler rand Gen_micheline.config).term
  in
  let shrink _ = assert false in
  QCheck2.Gen.make_primitive ~gen ~shrink

let gen_algo =
  QCheck2.Gen.oneofl Tezos_crypto.Signature.[ Ed25519; Secp256k1; P256 ]

let some_private_key =
  [
    "edsk3ayZxYsCj8jhgq8ioN4ETy99DqU8yksnq6s7fwcC95ZkDPypkC";
    "edsk41r5Z37fXTTFF5EwQ2Xonr7KsZ8tzvTB3dxCyQn3QsaFbE4MzZ";
    "edsk2w3gCWojHtZdkTFBsWZxvkqEo7zYMPrdmeujyMZsoPfBiFBxsf";
    "edsk3VhsjdEYFg7ZDTL5GB2vnX9Ee9k57GH2VM2emy2Z2A9dcYq8D9";
    "edsk3cWjLMKHahZVFEdvf46VkChebQPd1ctnHWJmLKiui7GHYxxxnK";
    "spsk2vcxPgevMHEU7LTDqg2ypar6gqmwSEz9aM4E3r4mW5B4HMTxDa";
    "spsk39ofnvbpah21bVLzEsEGZDFDZobmh18tde1VGi7CMJNYyFY9Dh";
    "spsk2Ze8YwUjYtmnevdZwQbj7Xyph7orZpTCfeQgyFxCXa3konv9aT";
    "spsk2GiCPV1oR3qnybxGwsHyCcgwFSXL1FVDxZyzDFYTUgrBVJkea5";
    "spsk2xJELkNQ3m7uS9Z6pTtctAmAg76REAME5b8puPvperSofRyU9y";
    "p2sk4APEttm8QwZFsaLq45YaYe3MTWPYwjoGcpvJ6NqRFvUNE8Btmv";
    "p2sk3KhuPgM96N136J2oZ39k4PLG6aUU8QLdMTtgYeShxd1fDtimw7";
    "p2sk3PE1VCKFTutZbfUBf8fwWkutzKhg3DSGgdCNrYLhnxLsPBPc86";
    "p2sk32wjLHQ1iyjnGnH3fPVCiY5MniQnBHBfDosiSeSjPzr7HFLEdM";
    "p2sk3RDMM4PkWDVjEcVTdLru2MifiboZLYrmxwCExUzTnLTuGSygrx";
  ]

let some_public_key =
  [
    "edpkvMUjmJu9CYyKBAjUV3jtU8Y89TemDAcD29bSNh393Bc8z8BH3t";
    "edpkvYT4Cbzg4BFDBMJKEsgUbFsrK8GPtpZcYVUufE88rkSoo7saXo";
    "edpkv5maJHfQAJdLyXefDEVFyK9tfbzKXSKmcT1YYqHc8MmKMphE6v";
    "edpkuGnyR1TBGbWznAvmBfMAFFk321v4bWsHYA5nR75uNyLinK23Go";
    "edpkv1cpS2U1VbviarXs3ZcSdYVXdcad4hVWk3SSGgjFXZUj8xnryJ";
    "sppk7ZT8R42AGSy672NHz9ps6Q4idqWYejAgMwqTWnyYAeq9XZEqWvZ";
    "sppk7a9FzG5f7quhhgiijyiNyktfP9CmqoP6sAa9iV7my2QErYDnk1k";
    "sppk7aVFzHeLSgsqMfKXP3PySiuXXTm7qju9SpqdXFasyNGNu2xiSBB";
    "sppk7busdHfsMPFsp2JYMgY8y3sqyAdbpowhS11BnfZ4RMomy4EQ8V2";
    "sppk7cZHYhQvxTDrsmokcUuJvRTp1amwQdYfGSrPxjL47GRGscDcLkq";
    "p2pk665znpiyPRWEwpu8tZ7JdNPipkfYpGUhYALjaS4Tm7F7wcx1iRs";
    "p2pk65U84BdxaihUbaJX9qdFzcx7QAJnVioLtvotYbZtehhU6tm1tcg";
    "p2pk64hc2xSWmKEztoTxkRtioDhG46qUfBuhvxpTA69kNEcGsLsPg8T";
    "p2pk66Y3TrkdsL8fYZq6cXBpLqgEVcSZDErYDctJWJc5ZzWkgech8Uj";
    "p2pk68CE92CP8tAjbp6LgNagDcns8acLLv3fPfgkHcBpvyQB4ka81BQ";
  ]

let some_public_key_hash =
  [
    "tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa";
    "tz1er74kx433vTtpYddGsf3dDt5piBZeeHyQ";
    "tz1McCh72NRhYmJBcWr3zDrLJAxnfR9swcFh";
    "tz1TmFPVZsGQ8MnrBJtnECJgkFUwLa6EWYDm";
    "tz1e8fEumaLvXXe5jV52gejCSt3mGodoKut9";
    "tz2W3Tvcm64GjcV2bipUynnEsctLFz5Z6yRa";
    "tz2JPgTWZZpxZZLqHMfS69UAy1UHm4Aw5iHu";
    "tz2CJBeWWLsUDjVUDqGZL6od3DeBCNzYXrXk";
    "tz2KC42yW9FXFMJpkUooae2NFYQsM5do3E8H";
    "tz2PPZ2WN4j92Rdx4NM7oW3HAp3x825HUyac";
    "tz3XeTwXXJeWNgVR3LqMcyBDdnxjbZ7TeEGH";
    "tz3fLwHKthqhTPK6Lar6CTXN1WbDETw1YpGB";
    "tz3eydffbLkjdVb8zx42BvxpGV87zaRnqL3r";
    "tz3hCsUiQDfneTgD7CSZDaUro8SA5aEhwCp2";
    "tz3Wazpbs4CFj78qv2KBJ8Z7HEyqk6ZPxwWZ";
  ]

let gen_secret_key =
  let open QCheck2.Gen in
  let+ sk = oneofl some_private_key in
  Tezos_crypto.Signature.Secret_key.of_b58check_exn sk

let gen_public_key =
  let open QCheck2.Gen in
  let+ pk = oneofl some_public_key in
  Tezos_crypto.Signature.Public_key.of_b58check_exn pk

let gen_public_key_hash =
  let open QCheck2.Gen in
  let+ pkh = oneofl some_public_key_hash in
  Tezos_crypto.Signature.Public_key_hash.of_b58check_exn pkh

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

let some_implicit_contract =
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
  ]

let gen_contract =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let gen_originated =
    let* public_key_hash = gen_public_key_hash in
    return (Contract.Implicit public_key_hash)
  in
  let gen_implict = return @@ Contract.Originated Protocol.Contract_hash.zero in
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
  let* parameters = gen_expr in
  return (Transaction { amount; destination; entrypoint; parameters })

let gen_update_consensus_key =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  let* public_key = gen_public_key in
  return (Update_consensus_key public_key)

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
  QCheck2.Gen.oneof
    [
      aux gen_delegation;
      aux gen_reveal;
      aux gen_set_deposits_limit;
      aux gen_transaction;
      aux gen_update_consensus_key;
    ]

type hidden_manager_operation_list =
  | HMOL :
      _ Protocol.Alpha_context.Kind.manager Protocol.Alpha_context.contents_list
      -> hidden_manager_operation_list

let gen_packed_contents_list =
  let open Protocol.Alpha_context in
  let open QCheck2.Gen in
  (* Do not make operation batches too large *)
  let* size = int_range 1 3 in
  let rec aux i =
    if i <= 1 then
      let* (HMO manager_operation) = gen_hidden_manager_operation in
      return (HMOL (Single manager_operation))
    else
      let* (HMO manager_operation) = gen_hidden_manager_operation in
      let* (HMOL contents_list) = aux (i - 1) in
      return (HMOL (Cons (manager_operation, contents_list)))
  in
  let* (HMOL contents_list) = aux size in
  return (Contents_list contents_list)

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
