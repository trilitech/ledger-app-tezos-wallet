#!/usr/bin/env python3
# Copyright 2024 Functori <contact@functori.com>

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from pathlib import Path
from typing import Callable

from utils.app import Screen, Screen_text, DEFAULT_ACCOUNT
from utils.message import Message
from utils.backend import StatusCode

test_path = Path(Path(__file__).stem)

def _sign_too_long(app, msg: str, navigate: Callable[[], None]):

    app.setup_expert_mode()

    message = Message.from_bytes(msg)

    data = app._sign(
        DEFAULT_ACCOUNT,
        message,
        with_hash=True,
        navigate=navigate)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()

def _sign_decodable_too_long(app, msg: str, path: Path):

    def navigate() -> None:
        app.navigate_until_text(Screen_text.Accept_risk, path / "clear_n_too_long_warning")
        app.navigate_until_text(Screen_text.Sign_accept, path / "summary")

    _sign_too_long(app, msg, navigate)

def _reject_too_long(
        app,
        msg: str,
        status_code: StatusCode,
        navigate: Callable[[], None]):

    app.setup_expert_mode()

    message = Message.from_bytes(msg)

    app._failing_signing(
        DEFAULT_ACCOUNT,
        message,
        with_hash=True,
        navigate=navigate,
        status_code=status_code)

    app.quit()


### Too long operation ###

basic_test_path = test_path / "basic"

## Operation (0): Reveal
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 1 XTZ
# Storage limit: 4
# Public key: p2pk66m3NQsd4n6LJWe9WMwx9WHeXwKmBaMwXX92WkMQCR99zmwk2PM
## Operation (1): Transaction
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 2 XTZ
# Storage limit: 7
# Amount: 3 XTZ
# Destination: tz3XMQscBFM9vPmpbYMavMmwxRMUWvWGZMQQ
# Entrypoint: update_config
# Parameter: Pair 5 True
## Operation (2): Delegation
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 3 XTZ
# Storage limit: 5
# Delegate: tz2W3Tvcm64GjcV2bipUynnEsctLFz5Z6yRa
## Operation (3): SR: send messages
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 4 XTZ
# Storage limit: 6
# Message (0):  6d65737361676530
# Message (1):  6d65737361676531
# Message (2):  6d65737361676532
# Message (3):  6d65737361676533
# Message (4):  6d65737361676534
# Message (5):  6d65737361676535
# Message (6):  6d65737361676536
# Message (7):  6d65737361676537
# Message (8):  6d65737361676538
# Message (9):  6d65737361676539
# Message (10): 6d6573736167653130
# Message (11): 6d6573736167653131
# Message (12): 6d6573736167653132
# Message (13): 6d6573736167653133
# Message (14): 6d6573736167653134
# Message (15): 6d6573736167653135
# Message (16): 6d6573736167653136
# Message (17): 6d6573736167653137
# Message (18): 6d6573736167653138
# Message (19): 6d6573736167653139
## Operation (4): Set deposit limit
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 1 XTZ
# Storage limit: 3
# Staking limit: 10 XTZ
basic_operation = "0300000000000000000000000000000000000000000000000000000000000000006b00ffdd6102321bc251e4a5190ad5b12b251069d9b4c0843d0b0104020320182716513907b6bab33f905396d031931c07e01bddd780780c1a56b9c086da6c00ffdd6102321bc251e4a5190ad5b12b251069d9b480897a0c0107c08db701000278eb8b6ab9a768579cd5146b480789650c83f28effff0d7570646174655f636f6e6669670000000607070005030a6e00ffdd6102321bc251e4a5190ad5b12b251069d9b4c08db7010d0105ff01ee572f02e5be5d097ba17369789582882e8abb87c900ffdd6102321bc251e4a5190ad5b12b251069d9b48092f4010e0106000000fa000000086d65737361676530000000086d65737361676531000000086d65737361676532000000086d65737361676533000000086d65737361676534000000086d65737361676535000000086d65737361676536000000086d65737361676537000000086d65737361676538000000086d65737361676539000000096d6573736167653130000000096d6573736167653131000000096d6573736167653132000000096d6573736167653133000000096d6573736167653134000000096d6573736167653135000000096d6573736167653136000000096d6573736167653137000000096d6573736167653138000000096d65737361676531397000ffdd6102321bc251e4a5190ad5b12b251069d9b4c0843d0f0103ff80ade204"

def test_sign_basic_too_long_operation(app):
    _sign_decodable_too_long(app, basic_operation, basic_test_path / "accept")

def test_reject_basic_too_long_operation_at_warning(app):
    path = basic_test_path / "reject_at_too_large_warning"

    def navigate() -> None:
        app.navigate_until_text(Screen_text.Sign_reject, path / "clear_n_too_long_warning")

    _reject_too_long(app, basic_operation, StatusCode.REJECT, navigate)

def test_reject_basic_too_long_operation_at_summary(app):
    path = basic_test_path / "reject_at_summary"

    def navigate() -> None:
        app.navigate_until_text(Screen_text.Accept_risk, path / "clear_n_too_long_warning")
        app.navigate_until_text(Screen_text.Sign_reject, path / "summary")

    _reject_too_long(app, basic_operation, StatusCode.REJECT, navigate)


### Different kind of too long operation ###

## Operation (0): Transaction
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 0 XTZ
# Storage limit: 0
# Amount: 10 XTZ
# Destination: tz1er74kx433vTtpYddGsf3dDt5piBZeeHyQ
## Operation (1): Transaction
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 1 XTZ
# Storage limit: 1
# Amount: 1 XTZ
# Destination: tz2PPZ2WN4j92Rdx4NM7oW3HAp3x825HUyac
## Operation (2): Transaction
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 2 XTZ
# Storage limit: 2
# Amount: 2 XTZ
# Destination: tz1Kp8NCAN5WWwvkWkMmQQXMRe68iURmoQ8w
## Operation (3): Transaction
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 3 XTZ
# Storage limit: 3
# Amount: 3 XTZ
# Destination: tz3fLwHKthqhTPK6Lar6CTXN1WbDETw1YpGB
## Operation (4): Transaction
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 4 XTZ
# Storage limit: 4
# Amount: 4 XTZ
# Destination: tz3eydffbLkjdVb8zx42BvxpGV87zaRnqL3r
## Operation (5): Transaction
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 5 XTZ
# Storage limit: 5
# Amount: 5 XTZ
# Destination: tz2JPgTWZZpxZZLqHMfS69UAy1UHm4Aw5iHu
def test_sign_too_long_operation_with_only_transactions(app):
    msg="0300000000000000000000000000000000000000000000000000000000000000006c00ffdd6102321bc251e4a5190ad5b12b251069d9b4000b010080ade2040000d2b3082b0fe03f0f7f39915cdba50e9d9b8ab057006c00ffdd6102321bc251e4a5190ad5b12b251069d9b4c0843d0c0101c0843d0001a55ae1858c1201514c97aa9122e77d3c4197a714006c00ffdd6102321bc251e4a5190ad5b12b251069d9b480897a0d010280897a000001e8e5519a315280a374c8765107979a6049de27006c00ffdd6102321bc251e4a5190ad5b12b251069d9b4c08db7010e0103c08db7010002d09584de879c4bbd3f494ed01b82e06a81e8e176006c00ffdd6102321bc251e4a5190ad5b12b251069d9b48092f4010f01048092f4010002cc8e146741cf31fc00123b8c26baf95c57421a3c006c00ffdd6102321bc251e4a5190ad5b12b251069d9b4c096b102100105c096b10200016e8874874d31c3fbd636e924d5a036a43ec8faa700"
    _sign_decodable_too_long(app, msg, test_path / "only_transactions")

## Operation (0): Proposals
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Period: 32
# Proposal (0):  ProtoDemoNoopsDemoNoopsDemoNoopsDemoNoopsDemo6XBoYp
# Proposal (1):  ProtoGenesisGenesisGenesisGenesisGenesisGenesk612im
# Proposal (2):  PrihK96nBAFSxVL1GLJTVhu9YnzkMFiBeuJRPA8NwuZVZCE1L6i
# Proposal (3):  Ps9mPmXaRzmzk35gbAYNCAw6UXdE2qoABTHbN2oEEc1qM7CwT9P
# Proposal (4):  PsBabyM1eUXZseaJdmXFApDSBqj8YBfwELoxZHHW77EMcAbbwAS
# Proposal (5):  PsCARTHAGazKbHtnKfLzQg3kms52kSRpgnDY982a9oYsSXRLQEb
# Proposal (6):  PsDELPH1Kxsxt8f9eWbxQeRxkjfbxoqM52jvs5Y5fBxWWh4ifpo
# Proposal (7):  PtEdo2ZkT9oKpimTah6x2embF25oss54njMuPzkJTEi5RqfdZFA
# Proposal (8):  PsFLorenaUUuikDWvMDr6fGBRG8kt3e3D3fHoXK1j1BFRxeSH4i
# Proposal (9):  PtGRANADsDU8R9daYKAgWnQYAJ64omN1o3KMGVCykShA97vQbvV
# Proposal (10): PtHangz2aRngywmSRGGvrcTyMbbdpWdpFKuS4uMWxg2RaH9i1qx
# Proposal (11): Psithaca2MLRFYargivpo7YvUr7wUDqyxrdhC5CQq78mRvimz6A
# Proposal (12): PtJakart2xVj7pYXJBXrqHgd82rdkLey5ZeeGwDgPp9rhQUbSqY
# Proposal (13): PtKathmankSpLLDALzWw7CGD2j2MtyveTwboEYokqUCP4a1LxMg
# Proposal (14): PtLimaPtLMwfNinJi9rCfDPWea8dFgTZ1MeJ9f1m2SRic6ayiwW
# Proposal (15): PtMumbai2TmsJHNGRkD8v8YDbtao7BLUC3wjASn1inAKLFCjaH1
# Proposal (16): PtNairobiyssHuh87hEhfVBGCVrK3WnS8Z2FT4ymB5tAa4r1nQf
# Proposal (17): ProxfordYmVfjWnRcgjWH36fW6PArwqykTFzotUxRs6gmTcZDuH
# Proposal (18): PtParisBxoLz5gzMmn3d9WBQNoPSZakgnkMC2VNuQ3KXfUtUQeZ
# Proposal (19): ProtoALphaALphaALphaALphaALphaALphaALphaALphaDdp3zK
def test_sign_too_long_operation_without_fee_or_amount(app):
    msg="0300000000000000000000000000000000000000000000000000000000000000000500ffdd6102321bc251e4a5190ad5b12b251069d9b400000020000002800bcd7db2d718ba94e85bd262681049852e1f58512aa552124330d657845c73b70bcd7ffca03f57e38453f0d3e84c302403c05357448b4c2daef8b3a8be3c69c1000000000000000000000000000000000000000000000000000000000000000038ecdef0cd08640f318a9b055f6b0d0c9ae030913a871d9b9d86fb846317da213d0b4bacb5c3e152a167da26fefc266bd3a0e14fc4e41e6c53623bf482833da23e5e3a606afab74a59ca09e333633e2770b6492c5e594455b71e9a2f0ea92afb40cab83d3f37a64da26b57ad3d0432ae881293a25169ada387bfc74a1cbf9e6ec7ad4f7a000e28e9eefc58de8ea1172de843242bd2e688779953d3416a44640b4596285c6871691e25196c6a8d26d90a3ac91375731e3926103c517a13a0ba56cbb944f74244ea2681981f25995f8ebba8ff6cee8c036892fe901cb760c4e39ece5f061e34b5a21feab8dbdfe755ef17e70c9f565464f067ac5e7c02be830a488424520cf9bbf0a42770204d95dcc1f11e404fdb3e90c84850c4cfdb50c5c4b9d0a3f07b8adfcf61f5ca60f244ca9a876e76cbad9140980f6c88d0bf900ac6d8d2ea9f23a1a1011091841b12e32ce2f8c3facff27feee58bb7c9e90567d11425d57ed88be5a69815e39386a33f7dcad391f5f507e03b376e499272c86c6cf2a7d8325f11da2ff36934a586439f085655a833f2ff6a12d15e83b951fb684326e0d9b8c2314cc05ffa3fc655a98bb87155be4cf7ce67fee6b594ea9302e8655df20bf44c7d64e3d7da27d925d10af535fb36cef0ad41930c7929773f4731eba137dbff6586a04802d3f513c65a444d9d4debe42b17e9e7273f8f6c118ea3f4e06e0bcd7b2cadcd87ecb0d5c50330fb59feed7432bffecede8a09a2b86d1527c6a5"
    _sign_decodable_too_long(app, msg, test_path / "without_fee_or_amount")


### Too long operation containing a too large number ###

too_large_test_path = test_path / "too_large"

# Operation (0): SR: send messages
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 4 XTZ
# Storage limit: 6
# Message (0):  6d65737361676530
# Message (1):  6d65737361676531
# Message (2):  6d65737361676532
# Message (3):  6d65737361676533
# Message (4):  6d65737361676534
# Message (5):  6d65737361676535
# Message (6):  6d65737361676536
# Message (7):  6d65737361676537
# Message (8):  6d65737361676538
# Message (9):  6d65737361676539
# Message (10): 6d6573736167653130
# Message (11): 6d6573736167653131
# Message (12): 6d6573736167653132
# Message (13): 6d6573736167653133
# Message (14): 6d6573736167653134
# Message (15): 6d6573736167653135
# Message (16): 6d6573736167653136
# Message (17): 6d6573736167653137
# Message (18): 6d6573736167653138
# Message (19): 6d6573736167653139
# Message (20): 6d6573736167653230
# Message (21): 6d6573736167653231
# Message (22): 6d6573736167653232
# Message (23): 6d6573736167653233
# Message (24): 6d6573736167653234
# Message (25): 6d6573736167653235
# Message (26): 6d6573736167653236
# Message (27): 6d6573736167653237
# Message (28): 6d6573736167653238
# Message (29): 6d6573736167653239
## Operation (1): Register global constant
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 5 XTZ
# Storage limit: 3
# Value: 115792089237316195423570985008687907853269984665640564039457584007913129639936
operation_with_too_large = "030000000000000000000000000000000000000000000000000000000000000000c900ffdd6102321bc251e4a5190ad5b12b251069d9b48092f4010b01060000017c000000086d65737361676530000000086d65737361676531000000086d65737361676532000000086d65737361676533000000086d65737361676534000000086d65737361676535000000086d65737361676536000000086d65737361676537000000086d65737361676538000000086d65737361676539000000096d6573736167653130000000096d6573736167653131000000096d6573736167653132000000096d6573736167653133000000096d6573736167653134000000096d6573736167653135000000096d6573736167653136000000096d6573736167653137000000096d6573736167653138000000096d6573736167653139000000096d6573736167653230000000096d6573736167653231000000096d6573736167653232000000096d6573736167653233000000096d6573736167653234000000096d6573736167653235000000096d6573736167653236000000096d6573736167653237000000096d6573736167653238000000096d65737361676532396f00ffdd6102321bc251e4a5190ad5b12b251069d9b4c096b1020c0103000000260080808080808080808080808080808080808080808080808080808080808080808080808020"

def test_sign_too_long_operation_with_too_large(app):
    path = too_large_test_path / "accept"

    def navigate() -> None:
        app.navigate_until_text(Screen_text.Accept_risk, path / "clear_n_too_long_warning")
        app.navigate_until_text(Screen_text.Accept_risk, path / "too_large_warning")
        app.navigate_until_text(Screen_text.Sign_accept, path / "blindsigning")

    _sign_too_long(app, operation_with_too_large, navigate)

def test_reject_too_long_operation_with_too_large_at_too_long_warning(app):
    path = too_large_test_path / "reject_at_too_long_warning"

    def navigate() -> None:
        app.navigate_until_text(Screen_text.Sign_reject, path / "clear_n_too_long_warning")

    _reject_too_long(app, operation_with_too_large, StatusCode.REJECT, navigate)

def test_reject_too_long_operation_with_too_large_at_too_large_warning(app):
    path = too_large_test_path / "reject_at_too_large_warning"

    def navigate() -> None:
        app.navigate_until_text(Screen_text.Accept_risk, path / "clear_n_too_long_warning")
        app.navigate_until_text(Screen_text.Sign_reject, path / "too_large_warning")

    _reject_too_long(app, operation_with_too_large, StatusCode.PARSE_ERROR, navigate)

def test_reject_too_long_operation_with_too_large_at_blindsigning(app):
    path = too_large_test_path / "reject_at_blindsigning"

    def navigate() -> None:
        app.navigate_until_text(Screen_text.Accept_risk, path / "clear_n_too_long_warning")
        app.navigate_until_text(Screen_text.Accept_risk, path / "too_large_warning")
        app.navigate_until_text(Screen_text.Sign_reject, path / "blindsigning")

    _reject_too_long(app, operation_with_too_large, StatusCode.REJECT, navigate)
