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

## APDU instructions in use by Tezos Ledger apps

| Instruction                     | Code | App | Prompt | Short description                                |
|---------------------------------|------|-----|--------|--------------------------------------------------|
| `INS_VERSION`                   | 0x00 | WB  | No     | Get version information for the ledger           |
| `INS_AUTHORIZE_BAKING`          | 0x01 | B   | Yes    | Authorize baking                                 |
| `INS_GET_PUBLIC_KEY`            | 0x02 | WB  | No     | Get the ledger’s internal public key             |
| `INS_PROMPT_PUBLIC_KEY`         | 0x03 | WB  | Yes    | Prompt for the ledger’s internal public key      |
| `INS_SIGN`                      | 0x04 | WB  | Yes    | Sign a message with the ledger’s key             |
| `INS_SIGN_UNSAFE`               | 0x05 | W   | Yes    | Sign a message with the ledger’s key (no hash)   |
| `INS_RESET`                     | 0x06 | B   | Yes    | Reset high water mark block level                |
| `INS_QUERY_AUTH_KEY`            | 0x07 | B   | No     | Get auth key                                     |
| `INS_QUERY_MAIN_HWM`            | 0x08 | B   | No     | Get current high water mark                      |
| `INS_GIT`                       | 0x09 | WB  | No     | Get the commit hash                              |
| `INS_SETUP`                     | 0x0a | B   | Yes    | Setup a baking address                           |
| `INS_QUERY_ALL_HWM`             | 0x0b | B   | No     | Get all high water mark information              |
| `INS_DEAUTHORIZE`               | 0x0c | B   | No     | Deauthorize baking                               |
| `INS_QUERY_AUTH_KEY_WITH_CURVE` | 0x0d | B   | No     | Get auth key and curve                           |
| `INS_HMAC`                      | 0x0e | B   | No     | Get the HMAC of a message                        |
| `INS_SIGN_WITH_HASH`            | 0x0f | WB  | Yes    | Sign a message with the ledger’s key (with hash) |

- B = Baking app, W = Wallet app

## Signing operations

There are 3 APDUs that deal with signing things. They use the Ledger’s
private key to sign messages sent. They are:

| Instruction          | Code | Parsing | Send hash |
|----------------------|------|---------|-----------|
| `INS_SIGN`           | 0x04 | Yes     | No        |
| `INS_SIGN_WITH_HASH` | 0x0f | Yes     | Yes       |

The difference between `INS_SIGN` and `INS_SIGN_WITH_HASH` is that the
latter returns both the signature *AND* the hash of the data (while
the former only returns the signature).

## Parsing

Todo
