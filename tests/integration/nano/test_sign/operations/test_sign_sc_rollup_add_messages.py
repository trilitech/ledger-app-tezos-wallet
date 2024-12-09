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

from utils.account import Account
from utils.backend import TezosBackend
from utils.message import ScRollupAddMessage
from utils.navigator import TezosNavigator


def test_sign_sc_rollup_add_messages(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check signing smart rollup add message"""

    message = ScRollupAddMessage(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 10000,
        counter = 2,
        gas_limit = 3,
        storage_limit = 4,
        message = [bytes.fromhex('012345'), bytes.fromhex('67'), bytes.fromhex('89abcdef')]
    )

    with backend.sign(account, message, with_hash=True) as result:
        tezos_navigator.accept_sign(snap_path=snapshot_dir)

    account.check_signature(
        message=message,
        with_hash=True,
        data=result.value
    )
