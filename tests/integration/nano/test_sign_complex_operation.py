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

"""Check signing complex operation"""

from pathlib import Path

from utils.app import Screen, TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import RawMessage

# Operation (0): Transaction
# Fee: 0.05 XTZ
# Storage limit: 45
# Amount: 0.24 XTZ
# Destination: KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT
# Entrypoint: do
# Parameter: CAR

## Operation (0): Origination
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 0.5 XTZ
# Storage limit: 4
# Balance: 1 XTZ
# Delegate: None
# Code: UNPACK mutez
# Storage: or key chest
## Operation (1): Transfer ticket
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 0.01 XTZ
# Storage limit: 5
# Contents: None
# Type: option nat
# Ticketer: tz1TmFPVZsGQ8MnrBJtnECJgkFUwLa6EWYDm
# Amount: 7
# Destination: tz3eydffbLkjdVb8zx42BvxpGV87zaRnqL3r
# Entrypoint: default

def test_sign_complex_operation(app: TezosAppScreen):
    """Check signing complex operation"""
    test_name = Path(__file__).stem

    app.assert_screen(Screen.HOME)
    app.setup_expert_mode()

    message = RawMessage("0300000000000000000000000000000000000000000000000000000000000000006d00ffdd6102321bc251e4a5190ad5b12b251069d9b4a0c21e040304c0843d0000000004050d036a000000060764035c038d9e00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e05040500000002030600000004056303620000591e842444265757d6a65e3670ca18b5e662f9c0070002cc8e146741cf31fc00123b8c26baf95c57421a3c0000000764656661756c74")

    data = app.sign(DEFAULT_ACCOUNT,
                    message,
                    with_hash=True,
                    path=test_name)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()
