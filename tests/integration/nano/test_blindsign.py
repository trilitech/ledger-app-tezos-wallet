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

def _blind_sign(app, msg: str, navigate: Callable[[], None]):

    app.setup_expert_mode()
    app.setup_blindsign_on()

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

def _blind_reject(
        app,
        msg: str,
        status_code: StatusCode,
        navigate: Callable[[], None]):

    app.setup_expert_mode()
    app.setup_blindsign_on()

    message = Message.from_bytes(msg)

    app._failing_signing(
        DEFAULT_ACCOUNT,
        message,
        with_hash=True,
        navigate=navigate,
        status_code=status_code)

    app.quit()


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

def test_blind_sign_basic_operation(app):
    path = basic_test_path / "accept"

    def navigate() -> None:
        app.navigate_until_text(Screen_text.Accept_risk, path / "clear")
        app.navigate_until_text(Screen_text.Sign_accept, path / "summary")

    _blind_sign(app, basic_operation, navigate)

def test_blind_reject_basic_operation_at_blind_warning(app):
    path = basic_test_path / "reject_at_blind_warning"

    def navigate() -> None:
        app.navigate_until_text(Screen_text.Sign_reject, path / "clear")

    _blind_reject(app, basic_operation, StatusCode.REJECT, navigate)

def test_blind_reject_basic_operation_at_summary(app):
    path = basic_test_path / "reject_at_summary"

    def navigate() -> None:
        app.navigate_until_text(Screen_text.Accept_risk, path / "clear")
        app.navigate_until_text(Screen_text.Sign_reject, path / "summary")

    _blind_reject(app, basic_operation, StatusCode.REJECT, navigate)

def test_no_blind_sign_basic_operation(app):
    path = basic_test_path / "not_blind"

    def navigate() -> None:
        app.navigate_until_text(Screen_text.Accept_risk, path / "clear_n_too_long_warning")
        app.navigate_until_text(Screen_text.Sign_accept, path / "summary")

    _blind_sign(app, basic_operation, navigate)


### Operation containing a too large number ###

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

def test_blind_sign_operation_with_too_large(app):
    path = too_large_test_path / "accept"

    def navigate() -> None:
        app.navigate_until_text(Screen_text.Accept_risk, path / "clear")
        app.navigate_until_text(Screen_text.Sign_accept, path / "blindsigning")

    _blind_sign(app, operation_with_too_large, navigate)

def test_blind_reject_operation_with_too_large_at_too_large_warning(app):
    path = too_large_test_path / "reject_at_too_large_warning"

    def navigate() -> None:
        app.navigate_until_text(Screen_text.Sign_reject, path / "clear")

    _blind_reject(app, operation_with_too_large, StatusCode.PARSE_ERROR, navigate)

def test_blind_reject_operation_with_too_large_at_blindsigning(app):
    path = too_large_test_path / "reject_at_blindsigning"

    def navigate() -> None:
        app.navigate_until_text(Screen_text.Accept_risk, path / "clear")
        app.navigate_until_text(Screen_text.Sign_reject, path / "blindsigning")

    _blind_reject(app, operation_with_too_large, StatusCode.REJECT, navigate)

def test_no_blind_sign_operation_with_too_large(app):
    path = too_large_test_path / "not_blind"

    def navigate() -> None:
        app.navigate_until_text(Screen_text.Accept_risk, path / "clear_n_too_large_warning")
        app.navigate_until_text(Screen_text.Sign_accept, path / "blindsigning")

    _blind_sign(app, operation_with_too_large, navigate)
