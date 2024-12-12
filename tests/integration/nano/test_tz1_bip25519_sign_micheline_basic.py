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

"""Check signing with bip25519"""

from pathlib import Path

from utils.account import Account, SigType
from utils.app import Screen, TezosAppScreen
from utils.message import MichelineExpr

def test_tz1_bip25519_sign_micheline_basic(app: TezosAppScreen):
    """Check signing with bip25519"""
    test_name = Path(__file__).stem

    app.assert_screen(Screen.HOME)

    account = Account("m/44'/1729'/0'/0'",
                      SigType.BIP32_ED25519,
                      "edpkumJgSsSxkpiB5hmTq6eZcrmc6BsJtLAhYceFTiziFqje4mongz")

    message = MichelineExpr([{'string': 'CACA'}, {'string': 'POPO'}, {'string': 'BOUDIN'}])

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
