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

"""Gathering of tests related to Transfer-ticket operations."""

from pathlib import Path

from conftest import requires_device
from utils.account import Account
from utils.backend import TezosBackend
from utils.message import TransferTicket
from utils.navigator import TezosNavigator


def test_sign_transfer_ticket(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check signing transfer ticket"""

    tezos_navigator.toggle_expert_mode()

    message = TransferTicket(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 10000,
        counter = 2,
        gas_limit = 3,
        storage_limit = 4,
        ticket_contents = {'prim': 'UNPAIR'},
        ticket_ty = {'prim': 'pair', 'args': [{'string': '1'}, {'int': 2}]},
        ticket_ticketer = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        ticket_amount = 1,
        destination = 'KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT'
    )

    with backend.sign(account, message, with_hash=True) as result:
        tezos_navigator.accept_sign(snap_path=snapshot_dir)

    account.check_signature(
        message=message,
        with_hash=True,
        data=result.value
    )

@requires_device("nanosp")
def test_nanosp_regression_potential_empty_screen(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check signing operation that display potentially empty screens"""

    tezos_navigator.toggle_expert_mode()

    message = TransferTicket(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 10000,
        counter = 2,
        gas_limit = 3,
        storage_limit = 4,
        ticket_contents = {'prim': 'UNPAIR'},
        ticket_ty = {'prim': 'pair', 'args': [{'string': '1'}, {'int': 2}]},
        ticket_ticketer = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        ticket_amount = 1,
        destination = 'KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT',
        entrypoint = 'S\n\nS\nS\nS'
    )

    with backend.sign(account, message, with_hash=True) as result:
        tezos_navigator.accept_sign(snap_path=snapshot_dir)

    account.check_signature(
        message=message,
        with_hash=True,
        data=result.value
    )
