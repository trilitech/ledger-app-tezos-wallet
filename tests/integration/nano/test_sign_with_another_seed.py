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

"""Check signing using another seed than [zebra*24]"""

from pathlib import Path

import pytest

from utils.account import Account, SigType
from utils.app import TezosAppScreen
from utils.message import Transaction

@pytest.mark.parametrize("seed", ["around dignity equal spread between young lawsuit interest climb wide that panther rather mom snake scene ecology reunion ice illegal brush"])
def test_sign_with_another_seed(app: TezosAppScreen):
    """Check signing using another seed than [zebra*24]"""
    test_name = Path(__file__).stem

    app.setup_expert_mode()

    account = Account("m/44'/1729'/0'/0'",
                      SigType.ED25519,
                      "edpkupntwMyERpYniuK1GDWquPaPU1wYsQgMirJPLGmC4Y5dMUsQNo")

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

    data = app.sign(account,
                    message,
                    with_hash=True,
                    path=test_name)

    app.checker.check_signature(
        account=account,
        message=message,
        with_hash=True,
        data=data)

    app.quit()
