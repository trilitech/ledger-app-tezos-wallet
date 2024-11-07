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

"""Check signing set consensus key"""

from pathlib import Path

from utils.app import Screen, TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import Message

# Operation (0): Set consensus key
# Fee: 0.01 XTZ
# Storage limit: 4
# Public key: edpkuXX2VdkdXzkN11oLCb8Aurdo1BTAtQiK8ZY9UPj2YMt3AHEpcY

def test_sign_set_consensus_key(app: TezosAppScreen):
    """Check signing set consensus key"""
    test_name = Path(__file__).stem

    app.assert_screen(Screen.HOME)

    message = Message.from_bytes("0300000000000000000000000000000000000000000000000000000000000000007200c921d4487c90b4472da6cc566a58d79f0d991dbf904e02030400747884d9abdf16b3ab745158925f567e222f71225501826fa83347f6cbe9c393")

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
