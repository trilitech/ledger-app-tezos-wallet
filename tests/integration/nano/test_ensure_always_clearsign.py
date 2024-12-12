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

"""Check clear signing never blindsign"""

from pathlib import Path

from utils.app import TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import Transaction

def test_ensure_always_clearsign(app: TezosAppScreen):
    """Check clear signing never blindsign"""
    test_name = Path(__file__).stem

    app.setup_expert_mode()

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
