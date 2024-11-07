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

"""Gathering of tests related to Key signatures."""

from pathlib import Path
import pytest

from utils.account import Account, SigType
from utils.app import Screen, TezosAppScreen
from utils.message import MichelineExpr, Transaction

def test_tz1_ed25519_sign_micheline_basic(app: TezosAppScreen):
    """Check signing with ed25519"""
    test_name = Path("test_tz1_ed25519_sign_micheline_basic")

    app.assert_screen(Screen.HOME)

    account = Account("m/44'/1729'/0'/0'",
                      SigType.ED25519,
                      "edpkuXX2VdkdXzkN11oLCb8Aurdo1BTAtQiK8ZY9UPj2YMt3AHEpcY")

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

def test_tz2_sign_micheline_basic(app: TezosAppScreen):
    """Check signing with secp256k1"""
    test_name = Path("test_tz2_sign_micheline_basic")

    account = Account("m/44'/1729'/0'/0'",
                      SigType.SECP256K1,
                      "sppk7bVy617DmGvXsMqcwsiLtnedTN2trUi5ugXcNig7en4rHJyunK1")

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

def test_tz3_sign_micheline_basic(app: TezosAppScreen):
    """Check signing with p256"""
    test_name = Path("test_tz3_sign_micheline_basic")

    account = Account("m/44'/1729'/0'/0'",
                      SigType.SECP256R1,
                      "p2pk67fq5pzuMMABZ9RDrooYbLrgmnQbLt8z7PTGM9mskf7LXS5tdBG")

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

def test_tz1_bip25519_sign_micheline_basic(app: TezosAppScreen):
    """Check signing with bip25519"""
    test_name = Path("test_tz1_bip25519_sign_micheline_basic")

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


@pytest.mark.parametrize("seed", ["around dignity equal spread between young lawsuit interest climb wide that panther rather mom snake scene ecology reunion ice illegal brush"])
def test_sign_with_another_seed(app: TezosAppScreen):
    """Check signing using another seed than [zebra*24]"""
    test_name = Path("test_sign_with_another_seed")

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
