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

"""Gathering of tests related to Batched operations."""

from pathlib import Path

from conftest import requires_device

from utils.app import Screen, TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import (
    OperationGroup,
    Origination,
    Transaction,
    TransferTicket
)

@requires_device("nanos")
def test_nanos_regression_batched_ops(app: TezosAppScreen, snapshot_dir: Path):
    """Check signing batch operation"""

    app.setup_expert_mode()

    message = OperationGroup([
        Transaction(
            source = 'tz1McCh72NRhYmJBcWr3zDrLJAxnfR9swcFh',
            fee = 390000,
            counter = 9,
            gas_limit = 0,
            storage_limit = 6,
            destination = 'tz1cfdVKpBb9VRBdny8BQ5RSK82UudAp2miM',
            amount = 20000,
            entrypoint = 'jean_bob',
            parameter = [{'prim':'Pair','args':[[],{'prim':'Right','args':[{'int':-76723569314251090535296646}]}]},{'prim':'Pair','args':[[{'prim':'Elt','args':[{'prim':'Unit','args':[]},{'prim':'Pair','args':[[{'prim':'Left','args':[{'prim':'Unit','args':[]}]}],{'prim':'Pair','args':[{'prim':'Left','args':[{'bytes':"03F01167865DC63DFEE0E31251329CEAB660D94606"}]},{'prim':'Pair','args':[{'bytes':"0107B21FCA96C5763F67B286752C7AAEFC5931D15A"},{'prim':'Unit','args':[]}]}]}]}]}],{'prim':'Right','args':[{'int':3120123370638446806591421154959427514880865200209654970345}]},]},{'prim':'Pair','args':[[],{'prim':'Left','args':[{'prim':'Some','args':[{'prim':'Unit','args':[]}]}]}]}]
        ),
        Transaction(
            source = 'tz2W3Tvcm64GjcV2bipUynnEsctLFz5Z6yRa',
            fee = 650000,
            counter = 6,
            gas_limit = 50,
            storage_limit = 2,
            destination = 'KT1CYT8oACUcCSNTu2qfgB4fj5bD7szYrpti',
            amount = 60000
        )
    ])

    data = app.sign(DEFAULT_ACCOUNT,
                    message,
                    with_hash=True,
                    path=snapshot_dir)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()

@requires_device("nanox")
def test_nanox_regression_batched_ops(app: TezosAppScreen, snapshot_dir: Path):
    """Check signing batch operation"""

    app.setup_expert_mode()

    message = OperationGroup([
        Transaction(
            source = 'tz1McCh72NRhYmJBcWr3zDrLJAxnfR9swcFh',
            fee = 390000,
            counter = 9,
            gas_limit = 0,
            storage_limit = 6,
            destination = 'tz1cfdVKpBb9VRBdny8BQ5RSK82UudAp2miM',
            amount = 20000,
            entrypoint = 'jean_bob',
            parameter = [{'prim':'Pair','args':[[],{'prim':'Right','args':[{'int':-76723569314251090535296646}]}]},{'prim':'Pair','args':[[{'prim':'Elt','args':[{'prim':'Unit','args':[]},{'prim':'Pair','args':[[{'prim':'Left','args':[{'prim':'Unit','args':[]}]}],{'prim':'Pair','args':[{'prim':'Left','args':[{'bytes':"03F01167865DC63DFEE0E31251329CEAB660D94606"}]},{'prim':'Pair','args':[{'bytes':"0107B21FCA96C5763F67B286752C7AAEFC5931D15A"},{'prim':'Unit','args':[]}]}]}]}]}],{'prim':'Right','args':[{'int':3120123370638446806591421154959427514880865200209654970345}]},]},{'prim':'Pair','args':[[],{'prim':'Left','args':[{'prim':'Some','args':[{'prim':'Unit','args':[]}]}]}]}]
        ),
        Transaction(
            source = 'tz2W3Tvcm64GjcV2bipUynnEsctLFz5Z6yRa',
            fee = 650000,
            counter = 6,
            gas_limit = 50,
            storage_limit = 2,
            destination = 'KT1CYT8oACUcCSNTu2qfgB4fj5bD7szYrpti',
            amount = 60000
        )
    ])

    data = app.sign(DEFAULT_ACCOUNT,
                    message,
                    with_hash=True,
                    path=snapshot_dir)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()

def test_sign_complex_operation(app: TezosAppScreen, snapshot_dir: Path):
    """Check signing complex operation"""

    app.assert_screen(Screen.HOME)
    app.setup_expert_mode()

    message = OperationGroup([
        Origination(
            source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
            fee = 500000,
            counter = 4,
            gas_limit = 3,
            storage_limit = 4,
            code = {'prim': 'UNPACK', 'args': [{'prim': 'mutez'}]},
            storage = {'prim': 'or', 'args': [{'prim': 'key'}, {'prim': 'chest'}]},
            balance = 1000000
        ),
        TransferTicket(
            source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
            fee = 10000,
            counter = 5,
            gas_limit = 4,
            storage_limit = 5,
            ticket_contents = {'prim': 'None'},
            ticket_ty = {'prim': 'option', 'args': [{'prim': 'nat'}]},
            ticket_ticketer = 'tz1TmFPVZsGQ8MnrBJtnECJgkFUwLa6EWYDm',
            ticket_amount = 7,
            destination = 'tz3eydffbLkjdVb8zx42BvxpGV87zaRnqL3r'
        )
    ])

    data = app.sign(DEFAULT_ACCOUNT,
                    message,
                    with_hash=True,
                    path=snapshot_dir)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()
