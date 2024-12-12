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

"""Check clear signing never blindsign"""

from pathlib import Path

from utils.app import TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import Message

# Operation (0): Transaction
# Fee: 0.01 XTZ
# Storage limit: 4
# Amount: 0 XTZ
# Destination: KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT
# Entrypoint: root
# Data: {pair "[" (pair "Z" (pair "Y" (pair "X" (pair "W" (pair "V" (pair "U" (pair "T" (pair "S" (pair "R" (pair "Q" (pair "P" (pair "O" (pair "N" (pair "M" (pair "L" (pair "K" (pair "J" (pair "I" (pair "H" (pair "G" (pair "F" (pair "E" (pair "D" (pair "C" (pair "B" {})))))))))))))))))))))))));pair 10 (pair 9 (pair 8 (pair 7 (pair 6 (pair 5 (pair 4 (pair 3 (pair 2 (pair 1 {})))))))))}

def test_ensure_always_clearsign(app: TezosAppScreen):
    """Check clear signing never blindsign"""
    test_name = Path(__file__).stem

    app.setup_expert_mode()

    message = Message.from_bytes("0300000000000000000000000000000000000000000000000000000000000000006c00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e0203040001000000000000000000000000000000000000000000ff01000001070200000102076501000000015b076501000000015a0765010000000159076501000000015807650100000001570765010000000156076501000000015507650100000001540765010000000153076501000000015207650100000001510765010000000150076501000000014f076501000000014e076501000000014d076501000000014c076501000000014b076501000000014a0765010000000149076501000000014807650100000001470765010000000146076501000000014507650100000001440765010000000143076501000000014202000000000765000a0765000907650008076500070765000607650005076500040765000307650002076500010200000000")

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
