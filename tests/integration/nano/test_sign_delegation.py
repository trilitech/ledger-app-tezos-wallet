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

"""Check signing delegation"""

from pathlib import Path

from utils.app import Screen, TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import RawMessage

# Operation (0): Delegation
# Fee: 0.2 XTZ
# Storage limit: 889
# Delegate: tz1TmFPVZsGQ8MnrBJtnECJgkFUwLa6EWYDm

def test_sign_delegation(app: TezosAppScreen):
    """Check signing delegation"""
    test_name = Path(__file__).stem

    app.assert_screen(Screen.HOME)

    message = RawMessage("0300000000000000000000000000000000000000000000000000000000000000006e01774d99da021b92d8c3dfc2e814c7658440319be2c09a0cf40509f906ff00591e842444265757d6a65e3670ca18b5e662f9c0")

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
