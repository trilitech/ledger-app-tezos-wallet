#!/usr/bin/env python3
# Copyright 2023 Functori <contact@functori.com>

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Check signing transaction"""

from pathlib import Path

from utils.app import Screen, ScreenText, TezosAppScreen, DEFAULT_ACCOUNT
from utils.backend import StatusCode
from utils.message import Message

test_path = Path(Path(__file__).stem)

# Operation (0): Transaction
# Source: tz2JPgTWZZpxZZLqHMfS69UAy1UHm4Aw5iHu
# Fee: 0.05 XTZ
# Storage limit: 45
# Amount: 0.24 XTZ
# Destination: KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT
# Entrypoint: do
# Parameter: CAR
def test_sign_transaction(app: TezosAppScreen):
    """Check signing transaction"""
    path = test_path / "basic"

    app.setup_expert_mode()

    message = Message.from_bytes("0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316")

    data = app.sign(DEFAULT_ACCOUNT,
                    message,
                    with_hash=True,
                    path=path)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()

# Operation (0): Transaction
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 0.01 XTZ
# Storage limit: 4
# Amount: 0 XTZ
# Destination: KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT
# Entrypoint: root
# Data: {pair "[" (pair "Z" (pair "Y" (pair "X" (pair "W" (pair "V" (pair "U" (pair "T" (pair "S" (pair "R" (pair "Q" (pair "P" (pair "O" (pair "N" (pair "M" (pair "L" (pair "K" (pair "J" (pair "I" (pair "H" (pair "G" (pair "F" (pair "E" (pair "D" (pair "C" (pair "B" {})))))))))))))))))))))))));pair 10 (pair 9 (pair 8 (pair 7 (pair 6 (pair 5 (pair 4 (pair 3 (pair 2 (pair 1 {})))))))))}
def test_reject_transaction(app: TezosAppScreen):
    """Check reject transaction"""
    path = test_path / "reject"

    app.setup_expert_mode()

    message = Message.from_bytes("0300000000000000000000000000000000000000000000000000000000000000006c00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e0203040001000000000000000000000000000000000000000000ff01000001070200000102076501000000015b076501000000015a0765010000000159076501000000015807650100000001570765010000000156076501000000015507650100000001540765010000000153076501000000015207650100000001510765010000000150076501000000014f076501000000014e076501000000014d076501000000014c076501000000014b076501000000014a0765010000000149076501000000014807650100000001470765010000000146076501000000014507650100000001440765010000000143076501000000014202000000000765000a0765000907650008076500070765000607650005076500040765000307650002076500010200000000")

    app.reject_signing(DEFAULT_ACCOUNT,
                       message,
                       with_hash=True,
                       path=path)

    app.quit()

# Operation (0): Transaction
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 0.5 XTZ
# Storage limit: 4
# Amount: 0.01 XTZ
# Destination: KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT
def test_sign_simple_transaction(app: TezosAppScreen):
    """Check sign not complex transaction"""
    path = test_path / "simple"

    app.setup_expert_mode()

    message = Message.from_bytes("0300000000000000000000000000000000000000000000000000000000000000006c00ffdd6102321bc251e4a5190ad5b12b251069d9b4a0c21e020304904e0100000000000000000000000000000000000000000000")

    data = app.sign(DEFAULT_ACCOUNT,
                    message,
                    with_hash=True,
                    path=path)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()

# Operation (0): Transaction
# Source: tz2JPgTWZZpxZZLqHMfS69UAy1UHm4Aw5iHu
# Fee: 0.05 XTZ
# Storage limit: 45
# Amount: 0.24 XTZ
# Destination: KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT
# Entrypoint: do
# Parameter: CAR
def test_too_complex_transaction(app: TezosAppScreen):
    """Check sign complex transaction"""
    path = test_path / "complex"
    app.assert_screen(Screen.HOME)

    message = Message.from_bytes("0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316")

    app._failing_signing(DEFAULT_ACCOUNT,
                         message,
                         with_hash=True,
                         navigate=(lambda: app.navigate_until_text(
                             ScreenText.BACK_HOME,
                             path)),
                         status_code=StatusCode.REJECT)

    app.quit()

# Operation (0): Transaction
# Source: tz2WmivuMG8MMRKMEmzKRMMxMApxZQWYNS4W
# Fee: 0.04 XTZ
# Storage limit: 2
# Amount: 1000 XTZ
# Destination: tz2CJBeWWLsUDjVUDqGZL6od3DeBCNzYXrXk
# Entrypoint: stake
# Parameter: Unit
def test_sign_stake_transaction(app: TezosAppScreen):
    """Check sign stake"""
    path = test_path / "stake"

    app.setup_expert_mode()

    message = Message.from_bytes("0300000000000000000000000000000000000000000000000000000000000000006c01f6552df4f5ff51c3d13347cab045cfdb8b9bd803c0b8020031028094ebdc0300012bad922d045c068660fabe19576f8506a1fa8fa3ff0600000002030b")

    data = app.sign(DEFAULT_ACCOUNT,
                    message,
                    with_hash=True,
                    path=path)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()

# Operation (0): Transaction
# Source: tz2WmivuMG8MMRKMEmzKRMMxMApxZQWYNS4W
# Fee: 0.04 XTZ
# Storage limit: 2
# Amount: 500 XTZ
# Destination: tz2CJBeWWLsUDjVUDqGZL6od3DeBCNzYXrXk
# Entrypoint: unstake
# Parameter: Unit
def test_sign_unstake_transaction(app: TezosAppScreen):
    """Check sign unstake"""
    path = test_path / "unstake"

    app.setup_expert_mode()

    message = Message.from_bytes("0300000000000000000000000000000000000000000000000000000000000000006c01f6552df4f5ff51c3d13347cab045cfdb8b9bd803c0b80200310280cab5ee0100012bad922d045c068660fabe19576f8506a1fa8fa3ff0700000002030b")

    data = app.sign(DEFAULT_ACCOUNT,
                    message,
                    with_hash=True,
                    path=path)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()

# Operation (0): Transaction
# Source: tz2WmivuMG8MMRKMEmzKRMMxMApxZQWYNS4W
# Fee: 0.04 XTZ
# Storage limit: 2
# Amount: 0 XTZ
# Destination: tz2CJBeWWLsUDjVUDqGZL6od3DeBCNzYXrXk
# Entrypoint: finalize_unstake
# Parameter: Unit
def test_sign_finalize_unstake_transaction(app: TezosAppScreen):
    """Check sign finalize_unstake"""
    path = test_path / "finalize_unstake"

    app.setup_expert_mode()

    message = Message.from_bytes("0300000000000000000000000000000000000000000000000000000000000000006c01f6552df4f5ff51c3d13347cab045cfdb8b9bd803c0b8020031020000012bad922d045c068660fabe19576f8506a1fa8fa3ff0800000002030b")

    data = app.sign(DEFAULT_ACCOUNT,
                    message,
                    with_hash=True,
                    path=path)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()

# Operation (0): Transaction
# Source: tz2WmivuMG8MMRKMEmzKRMMxMApxZQWYNS4W
# Fee: 0.04 XTZ
# Storage limit: 2
# Amount: 0 XTZ
# Destination: tz2CJBeWWLsUDjVUDqGZL6od3DeBCNzYXrXk
# Entrypoint: set_delegate_parameters
# Parameter: Pair 4000000 (Pair 20000000 Unit)
def test_sign_set_delegate_parameters_transaction(app: TezosAppScreen):
    """Check sign set delegate parameters"""
    path = test_path / "delegate_parameters"

    app.setup_expert_mode()

    message = Message.from_bytes("0300000000000000000000000000000000000000000000000000000000000000006c01f6552df4f5ff51c3d13347cab045cfdb8b9bd803c0b8020031020000012bad922d045c068660fabe19576f8506a1fa8fa3ff090000001007070080a4e80307070080b48913030b")

    data = app.sign(DEFAULT_ACCOUNT,
                    message,
                    with_hash=True,
                    path=path)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()
