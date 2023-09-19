# APDU

An APDU is sent by a client to the ledger hardware. This message tells
the ledger some operation to run. Most APDU messages will be
accompanied by an Accept / Reject prompt on the Ledger. Once the user
hits “Accept” the Ledger will issue a response to the Tezos client.
The basic format of the APDU request follows.

| Field   | Length   | Description                                                            |
|---------|----------|------------------------------------------------------------------------|
| *CLA*   | `1 byte` | Instruction class (always 0x80)                                        |
| *INS*   | `1 byte` | Instruction code (0x00-0x0f)                                           |
| *P1*    | `1 byte` | Index of the message (0x80 lor index = last index)                     |
| *P2*    | `1 byte` | Derivation type (0=ED25519, 1=SECP256K1, 2=SECP256R1, 3=BIP32_ED25519) |
| *LC*    | `1 byte` | Length of *CDATA*                                                      |
| *CDATA* | `<LC>`   | Payload containing instruction arguments                               |

Each APDU has a header of 5 bytes followed by some data. The APDU size
must not exceed `235 bytes`. The format of the data will depend on
which instruction is being used.

## Example

Here is an example of an APDU message from the ledger-app-tezos-wallet
tests:

> 0x8003000011048000002c800006c18000000080000000

This parses as:

| Field   | Value                                |
|---------|--------------------------------------|
| *CLA*   | 0x80                                 |
| *INS*   | 0x03                                 |
| *P1*    | 0x00                                 |
| *P2*    | 0x00                                 |
| *LC*    | 0x11 (17)                            |
| *CDATA* | 0x038000002c800006c18000000080000000 |

## Exception

| Exception                       | Code   | Short description                                        |
|---------------------------------|--------|----------------------------------------------------------|
| `EXC_WRONG_PARAM`               | 0x6B00 | Wrong parameter(s) *P1*-*P2*                             |
| `EXC_WRONG_LENGTH`              | 0x6C00 | Incorrect length.                                        |
| `EXC_INVALID_INS`               | 0x6D00 | Instruction code not supported or invalid.               |
| `EXC_WRONG_LENGTH_FOR_INS`      | 0x917E | Length of command string invalid.                        |
| `EXC_REJECT`                    | 0x6985 | Conditions of use not satisfied.                         |
| `EXC_PARSE_ERROR`               | 0x9405 | Problems in the data field.                              |
| `EXC_REFERENCED_DATA_NOT_FOUND` | 0x6A88 | Referenced data not found.                               |
| `EXC_WRONG_VALUES`              | 0x6A80 | The parameters in the data field are incorrect.          |
| `EXC_SECURITY`                  | 0x6982 | Security condition not satisfied.                        |
| `EXC_HID_REQUIRED`              | 0x6983 | Authentication method blocked.                           |
| `EXC_CLASS`                     | 0x6E00 | Class not supported.                                     |
| `EXC_MEMORY_ERROR`              | 0x9200 | Memory error.                                            |
| `EXC_UNEXPECTED_STATE`          | 0x9001 | The state of the application is unexpected.              |
| `EXC_UNEXPECTED_SIGN_STATE`     | 0x9002 | The state of the application at signature is unexpected. |
| `EXC_UNKNOWN`                   | 0x90FF | Unknown exception.                                        |

## APDU instructions in use by Tezos Ledger apps

| Instruction                     | Code | Prompt | Short description                                |
|---------------------------------|------|--------|--------------------------------------------------|
| `INS_VERSION`                   | 0x00 | No     | Get version information for the ledger           |
| `INS_GET_PUBLIC_KEY`            | 0x02 | No     | Get the ledger’s internal public key             |
| `INS_PROMPT_PUBLIC_KEY`         | 0x03 | Yes    | Prompt for the ledger’s internal public key      |
| `INS_SIGN`                      | 0x04 | Yes    | Sign a message with the ledger’s key             |
| `INS_GIT`                       | 0x09 | No     | Get the commit hash                              |
| `INS_SIGN_WITH_HASH`            | 0x0f | Yes    | Sign a message with the ledger’s key (with hash) |

## Instructions

### `INS_VERSION`

| *CLA* | *INS* |
|-------|-------|
| 0x80  | 0x00  |

Get version information for the ledger.

#### Input data

No input data.

#### Output data

| Length | Description              |
|--------|--------------------------|
| `1`    | Should be 0 for `wallet` |
| `1`    | The major version        |
| `1`    | The minor version        |
| `1`    | The patch version        |
| `2`    | Should be 0x9000         |

### `INS_GET_PUBLIC_KEY` / `INS_PROMPT_PUBLIC_KEY`

|                         | *CLA* | *INS* |
|-------------------------|-------|-------|
| `INS_GET_PUBLIC_KEY`    | 0x80  | 0x02  |
| `INS_PROMPT_PUBLIC_KEY` | 0x80  | 0x03  |

Get the ledger’s internal public key according to the `path`.

`INS_PROMPT_PUBLIC_KEY` will request confirmation before sending.

#### Input data

| Length       | Name   | Description       |
|--------------|--------|-------------------|
| `<variable>` | `path` | The mnemonic path |

#### Output data

| Length     | Description             |
|------------|-------------------------|
| `1`        | The public key `length` |
| `<length>` | The public key          |
| `2`        | Should be 0x9000        |

### `INS_SIGN` / `INS_SIGN_WITH_HASH`

|                      | *CLA* | *INS* |
|----------------------|-------|-------|
| `INS_SIGN`           | 0x80  | 0x04  |
| `INS_SIGN_WITH_HASH` | 0x80  | 0x0f  |

After accepting the request, it will sign the `message` with the key
corresponding to the `path`.

If blind signing is disabled, only the hash of the `message` will be
displayed. Otherwise, the message will be parsed and displayed (see
section [Parsing](#parsing)).

These instructions will require several APDUs.

#### First APDU

This APDU corresponds to the mnemonic `path` which, together with the
mnemonic, will define the public key with which the `message` will be
signed.

##### Input data

| Length       | Name   | Description       |
|--------------|--------|-------------------|
| `<variable>` | `path` | The mnemonic path |

##### Output data

| Length | Description      |
|--------|------------------|
| `2`    | Should be 0x9000 |

#### Other APDU

These APDUs correspond to the `message` that needs to be signed.

##### Input data

| Length       | Name      | Description         |
|--------------|-----------|---------------------|
| `<variable>` | `message` | The message to sign |

##### Output data

All these APDUs should respond with a success RAPDU as follows:

| Length | Description      |
|--------|------------------|
| `2`    | Should be 0x9000 |

Except for the last one, which will reply, after confirmation, with
the `message` signed in a RAPDU as follows:

| Length       | Description                                               |
|--------------|-----------------------------------------------------------|
| `32`         | The hash (Only with the instruction `INS_SIGN_WITH_HASH`) |
| `<variable>` | The signed hash                                           |
| `2`          | Should be 0x9000                                          |

### `INS_GIT`

| *CLA* | *INS* |
|-------|-------|
| 0x80  | 0x09  |

Get the commit hash.

#### Input data

No input data.

#### Output data

| Length       | Description      |
|--------------|------------------|
| `<variable>` | The commit       |
| `2`          | Should be 0x9000 |

## Parsing

The current version of the application is compatible with the protocol
Nairobi.

There are 2 kind of data that can be parsed when blind signing is
disable:
 - Michelson
 - Operations

If the data is invalid or too large, the analysis may fail with an
`EXC_UNEXPECTED_STATE` exception or an `EXC_PARSE_ERROR` exception.

### Michelson

Currently, all valid Michelson can be parsed up to the Nairobi
protocol (See the
[API](https://tezos.gitlab.io/nairobi/michelson.html)).


### Operations

There are currently fifteen operations supported in the Ledger, they
need to be valid to be parsed (See the
[API](https://tezos.gitlab.io/shell/p2p_api.html)):
- The `Proposal` operation enables delegates to submit (also known as
  to “inject”) protocol amendment proposals, or to up-vote previously
  submitted proposals, during the Proposal period.
- The `Ballot` operation enables delegates to participate in the
  Exploration and Promotion periods. Delegates use this operation to
  vote for (Yea), against (Nay), or to side with the majority (Pass),
  when examining a protocol amendment proposal.
- The `Reveal` operation reveals the public key of the sending
  manager. Knowing this public key is indeed necessary to check the
  signature of future operations signed by this manager.
- The `Transaction` operation allows users to either transfer tez
  between accounts and/or to invoke a smart contract.
- The `Delegation` operation allows users to delegate their stake to a
  delegate (a baker), or to register themselves as delegates.
- The `Update_consensus_key` operation allows users to delegate the
  responsibility of signing blocks and consensus-related operations to
  another account. Note that consensus keys cannot be BLS public keys.
- The `Origination` operation is used to originate, that is to deploy,
  smart contracts in the Tezos blockchain.
- The `Set_deposits_limit` operation enables delegates to adjust the
  amount of stake a delegate has locked in bonds.
- The `Increase_paid_storage` operation allows a sender to increase
  the paid storage of some previously deployed contract.
- The `Register_global_constant` operation for registering global
  constants.
- The `Smart_rollup_originate` operation is used to originate, that
  is, to deploy smart rollups in the Tezos blockchain.
- The `Smart_rollup_add_messages` operation is used to add messages to
  the inbox shared by all the smart rollups originated in the Tezos
  blockchain. These messages are interpreted by the smart rollups
  according to their specific semantics.
- The `Smart_rollup_execute_outbox_message` operation is used to enact
  a transaction from a smart rollup to a smart contract, as authorized
  by a cemented commitment. The targeted smart contract can determine
  if it is called by a smart rollup using the SENDER Michelson
  instruction.
- The `Transfer_ticket` operation allows implicit accounts to transfer
  existing tickets from their wallets to other implicit accounts or
  smart contracts.
- The `Failing_noop` operation implements a No-op, which always fails
  at application time, and should never appear in applied blocks. This
  operation allows end-users to sign arbitrary messages which have no
  computational semantics.
