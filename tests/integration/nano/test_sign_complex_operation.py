#!/usr/bin/env python3
# Copyright 2024 Functori <contact@functori.com>

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Check signing complex operation"""

from pathlib import Path

from utils.app import Screen, TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import (
    OperationGroup,
    Origination,
    TransferTicket
)

def test_sign_complex_operation(app: TezosAppScreen):
    """Check signing complex operation"""
    test_name = Path(__file__).stem

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
                    path=test_name)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()
