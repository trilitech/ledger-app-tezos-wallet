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

"""Gathering of tests related to Delegation operations."""

from pathlib import Path

from utils.account import Account
from utils.app import TezosAppScreen
from utils.message import Delegation

def test_sign_delegation(app: TezosAppScreen, account: Account, snapshot_dir: Path):
    """Check signing delegation"""

    message = Delegation(
        source = 'tz2KC42yW9FXFMJpkUooae2NFYQsM5do3E8H',
        fee = 200000,
        counter = 756,
        gas_limit = 9,
        storage_limit = 889,
        delegate = 'tz1TmFPVZsGQ8MnrBJtnECJgkFUwLa6EWYDm'
    )

    with app.backend.sign(account, message, with_hash=True) as result:
        app.accept_sign(snap_path=snapshot_dir)

    account.check_signature(
        message=message,
        with_hash=True,
        data=result.value
    )
