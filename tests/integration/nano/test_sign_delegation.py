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
from utils.message import Delegation

def test_sign_delegation(app: TezosAppScreen):
    """Check signing delegation"""
    test_name = Path(__file__).stem

    app.assert_screen(Screen.HOME)

    message = Delegation(
        source = 'tz2KC42yW9FXFMJpkUooae2NFYQsM5do3E8H',
        fee = 200000,
        counter = 756,
        gas_limit = 9,
        storage_limit = 889,
        delegate = 'tz1TmFPVZsGQ8MnrBJtnECJgkFUwLa6EWYDm'
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
