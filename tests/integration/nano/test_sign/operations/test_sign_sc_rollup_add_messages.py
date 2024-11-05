#!/usr/bin/env python3
# Copyright 2024 Functori <contact@functori.com>
# Copyright 2024 Trilitech <contact@trili.tech>

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Gathering of tests related to Smart-rollup Add-message operations."""

from pathlib import Path

from utils.app import Screen, TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import ScRollupAddMessage

def test_sign_sc_rollup_add_messages(app: TezosAppScreen):
    """Check signing smart rollup add message"""
    test_name = Path(__file__).stem

    app.assert_screen(Screen.HOME)

    message = ScRollupAddMessage(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 10000,
        counter = 2,
        gas_limit = 3,
        storage_limit = 4,
        message = [bytes.fromhex('012345'), bytes.fromhex('67'), bytes.fromhex('89abcdef')]
    )

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
