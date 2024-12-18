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

"""Check signing batch operation"""

from pathlib import Path

from conftest import requires_device

from utils.app import TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import OperationGroup, Transaction

@requires_device("nanox")
def test_nanox_regression_batched_ops(app: TezosAppScreen):
    """Check signing batch operation"""
    test_name = Path(__file__).stem

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
                    path=test_name)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()
