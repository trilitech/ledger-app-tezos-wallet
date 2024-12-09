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

"""Gathering of tests related to Transaction operations."""

from pathlib import Path

from utils.account import Account
from utils.backend import StatusCode, TezosBackend
from utils.message import Transaction
from utils.navigator import ScreenText, TezosNavigator


def test_sign_transaction(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check signing transaction"""

    tezos_navigator.toggle_expert_mode()

    message = Transaction(
        source = 'tz2JPgTWZZpxZZLqHMfS69UAy1UHm4Aw5iHu',
        fee = 50000,
        counter = 8,
        gas_limit = 54,
        storage_limit = 45,
        destination = 'KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT',
        amount = 240000,
        entrypoint = 'do',
        parameter = {'prim': 'CAR'}
    )

    with backend.sign(account, message, with_hash=True) as result:
        tezos_navigator.accept_sign(snap_path=snapshot_dir)

    account.check_signature(
        message=message,
        with_hash=True,
        data=result.value
    )

def test_reject_transaction(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check reject transaction"""

    tezos_navigator.toggle_expert_mode()

    message = Transaction(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 10000,
        counter = 2,
        gas_limit = 3,
        storage_limit = 4,
        destination = 'KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT',
        amount = 0,
        entrypoint = 'root',
        parameter = [{'prim':'pair','args':[{'string':"["},{'prim':'pair','args':[{'string':"Z"},{'prim':'pair','args':[{'string':"Y"},{'prim':'pair','args':[{'string':"X"},{'prim':'pair','args':[{'string':"W"},{'prim':'pair','args':[{'string':"V"},{'prim':'pair','args':[{'string':"U"},{'prim':'pair','args':[{'string':"T"},{'prim':'pair','args':[{'string':"S"},{'prim':'pair','args':[{'string':"R"},{'prim':'pair','args':[{'string':"Q"},{'prim':'pair','args':[{'string':"P"},{'prim':'pair','args':[{'string':"O"},{'prim':'pair','args':[{'string':"N"},{'prim':'pair','args':[{'string':"M"},{'prim':'pair','args':[{'string':"L"},{'prim':'pair','args':[{'string':"K"},{'prim':'pair','args':[{'string':"J"},{'prim':'pair','args':[{'string':"I"},{'prim':'pair','args':[{'string':"H"},{'prim':'pair','args':[{'string':"G"},{'prim':'pair','args':[{'string':"F"},{'prim':'pair','args':[{'string':"E"},{'prim':'pair','args':[{'string':"D"},{'prim':'pair','args':[{'string':"C"},{'prim':'pair','args':[{'string':"B"},[]]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]},{'prim':'pair','args':[{'int':10},{'prim':'pair','args':[{'int':9},{'prim':'pair','args':[{'int':8},{'prim':'pair','args':[{'int':7},{'prim':'pair','args':[{'int':6},{'prim':'pair','args':[{'int':5},{'prim':'pair','args':[{'int':4},{'prim':'pair','args':[{'int':3},{'prim':'pair','args':[{'int':2},{'prim':'pair','args':[{'int':1},[]]}]}]}]}]}]}]}]}]}]}]
    )

    with StatusCode.REJECT.expected():
        with backend.sign(account, message, with_hash=True):
            tezos_navigator.reject_sign(snap_path=snapshot_dir)

def test_sign_simple_transaction(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check sign not complex transaction"""

    tezos_navigator.toggle_expert_mode()

    message = Transaction(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 500000,
        counter = 2,
        gas_limit = 3,
        storage_limit = 4,
        destination = 'KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT',
        amount = 10000
    )

    with backend.sign(account, message, with_hash=True) as result:
        tezos_navigator.accept_sign(snap_path=snapshot_dir)

    account.check_signature(
        message=message,
        with_hash=True,
        data=result.value
    )

def test_too_complex_transaction(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check sign complex transaction"""

    message = Transaction(
        source = 'tz2JPgTWZZpxZZLqHMfS69UAy1UHm4Aw5iHu',
        fee = 50000,
        counter = 8,
        gas_limit = 54,
        storage_limit = 45,
        destination = 'KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT',
        amount = 240000,
        entrypoint = 'do',
        parameter = {'prim': 'CAR'}
    )

    with StatusCode.REJECT.expected():
        with backend.sign(account, message, with_hash=True):
            tezos_navigator.expert_reject_sign(snap_path=snapshot_dir)

def test_sign_stake_transaction(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check sign stake"""

    tezos_navigator.toggle_expert_mode()

    message = Transaction(
        source = 'tz2WmivuMG8MMRKMEmzKRMMxMApxZQWYNS4W',
        fee = 40000,
        counter = 0,
        gas_limit = 49,
        storage_limit = 2,
        destination = 'tz2CJBeWWLsUDjVUDqGZL6od3DeBCNzYXrXk',
        amount = 1000000000,
        entrypoint = 'stake',
    )

    with backend.sign(account, message, with_hash=True) as result:
        tezos_navigator.accept_sign(snap_path=snapshot_dir)

    account.check_signature(
        message=message,
        with_hash=True,
        data=result.value
    )

def test_sign_unstake_transaction(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check sign unstake"""

    tezos_navigator.toggle_expert_mode()

    message = Transaction(
        source = 'tz2WmivuMG8MMRKMEmzKRMMxMApxZQWYNS4W',
        fee = 40000,
        counter = 0,
        gas_limit = 49,
        storage_limit = 2,
        destination = 'tz2CJBeWWLsUDjVUDqGZL6od3DeBCNzYXrXk',
        amount = 500000000,
        entrypoint = 'unstake'
    )

    with backend.sign(account, message, with_hash=True) as result:
        tezos_navigator.accept_sign(snap_path=snapshot_dir)

    account.check_signature(
        message=message,
        with_hash=True,
        data=result.value
    )

def test_sign_finalize_unstake_transaction(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check sign finalize_unstake"""

    tezos_navigator.toggle_expert_mode()

    message = Transaction(
        source = 'tz2WmivuMG8MMRKMEmzKRMMxMApxZQWYNS4W',
        fee = 40000,
        counter = 0,
        gas_limit = 49,
        storage_limit = 2,
        destination = 'tz2CJBeWWLsUDjVUDqGZL6od3DeBCNzYXrXk',
        amount = 0,
        entrypoint = 'finalize_unstake'
    )

    with backend.sign(account, message, with_hash=True) as result:
        tezos_navigator.accept_sign(snap_path=snapshot_dir)

    account.check_signature(
        message=message,
        with_hash=True,
        data=result.value
    )

def test_sign_set_delegate_parameters_transaction(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check sign set delegate parameters"""

    tezos_navigator.toggle_expert_mode()

    message = Transaction(
        source = 'tz2WmivuMG8MMRKMEmzKRMMxMApxZQWYNS4W',
        fee = 40000,
        counter = 0,
        gas_limit = 49,
        storage_limit = 2,
        destination = 'tz2CJBeWWLsUDjVUDqGZL6od3DeBCNzYXrXk',
        amount = 0,
        entrypoint = 'delegate_parameters',
        parameter = {'prim': 'Pair',
                     'args': [
                         {'int': 4000000},
                         {'prim': 'Pair',
                          'args': [
                              {'int': 20000000},
                              {'prim': 'Unit'}
                          ]}
                     ]}
    )

    with backend.sign(account, message, with_hash=True) as result:
        tezos_navigator.accept_sign(snap_path=snapshot_dir)

    account.check_signature(
        message=message,
        with_hash=True,
        data=result.value
    )

def test_sign_with_long_hash(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check signing transaction with a long destination hash"""

    tezos_navigator.toggle_expert_mode()

    message = Transaction(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 10000,
        counter = 2,
        gas_limit = 3,
        storage_limit = 4,
        destination = 'KT1GW4QHn66m7WWWMWCMNaWmGYpCRbg5ahwU',
        amount = 0,
        entrypoint = 'root',
        parameter = {'int': 0}
    )

    with backend.sign(account, message, with_hash=True) as result:
        tezos_navigator.accept_sign(snap_path=snapshot_dir)

    account.check_signature(
        message=message,
        with_hash=True,
        data=result.value
    )

def test_ensure_always_clearsign(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check clear signing never blindsign"""

    tezos_navigator.toggle_expert_mode()

    message = Transaction(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 10000,
        counter = 2,
        gas_limit = 3,
        storage_limit = 4,
        destination = 'KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT',
        amount = 0,
        entrypoint = 'root',
        parameter = [{'prim':'pair','args':[{'string':"["},{'prim':'pair','args':[{'string':"Z"},{'prim':'pair','args':[{'string':"Y"},{'prim':'pair','args':[{'string':"X"},{'prim':'pair','args':[{'string':"W"},{'prim':'pair','args':[{'string':"V"},{'prim':'pair','args':[{'string':"U"},{'prim':'pair','args':[{'string':"T"},{'prim':'pair','args':[{'string':"S"},{'prim':'pair','args':[{'string':"R"},{'prim':'pair','args':[{'string':"Q"},{'prim':'pair','args':[{'string':"P"},{'prim':'pair','args':[{'string':"O"},{'prim':'pair','args':[{'string':"N"},{'prim':'pair','args':[{'string':"M"},{'prim':'pair','args':[{'string':"L"},{'prim':'pair','args':[{'string':"K"},{'prim':'pair','args':[{'string':"J"},{'prim':'pair','args':[{'string':"I"},{'prim':'pair','args':[{'string':"H"},{'prim':'pair','args':[{'string':"G"},{'prim':'pair','args':[{'string':"F"},{'prim':'pair','args':[{'string':"E"},{'prim':'pair','args':[{'string':"D"},{'prim':'pair','args':[{'string':"C"},{'prim':'pair','args':[{'string':"B"},[]]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]}]},{'prim':'pair','args':[{'int':10},{'prim':'pair','args':[{'int':9},{'prim':'pair','args':[{'int':8},{'prim':'pair','args':[{'int':7},{'prim':'pair','args':[{'int':6},{'prim':'pair','args':[{'int':5},{'prim':'pair','args':[{'int':4},{'prim':'pair','args':[{'int':3},{'prim':'pair','args':[{'int':2},{'prim':'pair','args':[{'int':1},[]]}]}]}]}]}]}]}]}]}]}]
    )

    with backend.sign(account, message, with_hash=True) as result:
        tezos_navigator.accept_sign(snap_path=snapshot_dir)

    account.check_signature(
        message=message,
        with_hash=True,
        data=result.value
    )
