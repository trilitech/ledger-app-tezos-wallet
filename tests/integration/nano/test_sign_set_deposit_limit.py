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

"""Check signing set deposit limit"""

from pathlib import Path

from utils.app import Screen, TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import RawMessage

# Operation (0): Set deposit limit
# Fee: 0.06 XTZ
# Storage limit: 4
# Staking limit: 0.02 XTZ

def test_sign_set_deposit_limit(app: TezosAppScreen):
    """Check signing set deposit limit"""
    test_name = Path(__file__).stem

    app.assert_screen(Screen.HOME)

    message = RawMessage("03000000000000000000000000000000000000000000000000000000000000000070027c252d3806e6519ed064026bdb98edf866117331e0d40304f80204ffa09c01")

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
