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

"""Check signing increase paid storage"""

from pathlib import Path

from utils.app import Screen, TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import RawMessage

# Operation (0): Increase paid storage
# Fee: 0.01 XTZ
# Storage limit: 4
# Amount: 5
# Destination: KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT

def test_sign_increase_paid_storage(app: TezosAppScreen):
    """Check signing increase paid storage"""
    test_name = Path(__file__).stem

    app.assert_screen(Screen.HOME)

    message = RawMessage("0300000000000000000000000000000000000000000000000000000000000000007100ffdd6102321bc251e4a5190ad5b12b251069d9b4904e0203040501000000000000000000000000000000000000000000")

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
