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

"""Gathering of tests related to APDU checks."""

from pathlib import Path

from utils.account import Account, SigType
from utils.app import Screen, TezosAppScreen, DEFAULT_ACCOUNT
from utils.backend import Cla, Index, Ins, StatusCode
from utils.message import Transaction

def test_regression_continue_after_reject(app: TezosAppScreen):
    """Check the app still runs after rejects signing"""
    test_name = "test_regression_continue_after_reject"

    def make_path(name: str) -> Path:
        return Path(test_name) / name

    app.setup_expert_mode()

    app.reject_public_key(DEFAULT_ACCOUNT, make_path("reject_public_key"))

    app.assert_screen(Screen.HOME)

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

    app.reject_signing(DEFAULT_ACCOUNT,
                       message,
                       with_hash=True,
                       path=make_path("reject_signing"))

    data = app.backend.get_public_key(DEFAULT_ACCOUNT, with_prompt=False)

    app.checker.check_public_key(DEFAULT_ACCOUNT, data)

    app.quit()

def test_change_sign_instruction(app: TezosAppScreen):
    """Check signing instruction changes behaviour"""

    app.assert_screen(Screen.HOME)

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
    payload=bytes(message)

    app.backend._ask_sign(Ins.SIGN_WITH_HASH, DEFAULT_ACCOUNT)

    with app.expect_apdu_failure(StatusCode.INVALID_INS):
        app.backend._continue_sign(Ins.SIGN,
                                   payload,
                                   last=True)

    app.assert_screen(Screen.HOME)

    app.backend._ask_sign(Ins.SIGN, DEFAULT_ACCOUNT)

    with app.expect_apdu_failure(StatusCode.INVALID_INS):
        app.backend._continue_sign(Ins.SIGN_WITH_HASH,
                                   payload,
                                   last=True)

    app.quit()

def test_mixing_command(app: TezosAppScreen):
    """Check that mixing instruction fails"""

    app.assert_screen(Screen.HOME)

    app.backend._ask_sign(Ins.SIGN, DEFAULT_ACCOUNT)
    with app.expect_apdu_failure(StatusCode.UNEXPECTED_STATE):
        app.backend.version()

    app.assert_screen(Screen.HOME)

    app.backend._ask_sign(Ins.SIGN_WITH_HASH, DEFAULT_ACCOUNT)
    with app.expect_apdu_failure(StatusCode.UNEXPECTED_STATE):
        app.backend._ask_sign(Ins.SIGN, DEFAULT_ACCOUNT)

    app.assert_screen(Screen.HOME)

    app.backend._ask_sign(Ins.SIGN, DEFAULT_ACCOUNT)
    with app.expect_apdu_failure(StatusCode.UNEXPECTED_STATE):
        app.backend._ask_sign(Ins.SIGN_WITH_HASH, DEFAULT_ACCOUNT)

    app.assert_screen(Screen.HOME)

    app.backend._ask_sign(Ins.SIGN, DEFAULT_ACCOUNT)
    with app.expect_apdu_failure(StatusCode.UNEXPECTED_STATE):
        app.backend.get_public_key(DEFAULT_ACCOUNT, with_prompt=True)

    app.assert_screen(Screen.HOME)

    app.backend._ask_sign(Ins.SIGN, DEFAULT_ACCOUNT)
    with app.expect_apdu_failure(StatusCode.UNEXPECTED_STATE):
        app.backend.get_public_key(DEFAULT_ACCOUNT, with_prompt=False)

    app.assert_screen(Screen.HOME)

    app.backend._ask_sign(Ins.SIGN, DEFAULT_ACCOUNT)
    with app.expect_apdu_failure(StatusCode.UNEXPECTED_STATE):
        app.backend.git()

    app.quit()

def test_wrong_index(app: TezosAppScreen):
    """Check wrong apdu index behaviour"""
    for ins in [Ins.GET_PUBLIC_KEY,
                Ins.PROMPT_PUBLIC_KEY]:
        for index in [Index.OTHER,
                      Index.LAST]:

            app.assert_screen(Screen.HOME)

            with app.expect_apdu_failure(StatusCode.WRONG_PARAM):
                app.backend._exchange(ins,
                                      index=index,
                                      sig_type=DEFAULT_ACCOUNT.sig_type,
                                      payload=DEFAULT_ACCOUNT.path)

    app.quit()

def test_wrong_derivation_type(app: TezosAppScreen):
    """Check wrong derivation type behaviour"""
    account = Account("m/44'/1729'/0'/0'", 0x04, "__unused__")

    for sender in [lambda account: app.backend.get_public_key(account, with_prompt=False),
                   lambda account: app.backend.get_public_key(account, with_prompt=True),
                   lambda account: app.backend._ask_sign(Ins.SIGN, account),
                   lambda account: app.backend._ask_sign(Ins.SIGN_WITH_HASH, account)]:

        app.assert_screen(Screen.HOME)

        with app.expect_apdu_failure(StatusCode.WRONG_PARAM):
            sender(account)

    app.quit()

def test_wrong_derivation_path(app: TezosAppScreen):
    """Check wrong derivation path behaviour"""
    wrong_number_index_account = Account(
        bytes.fromhex("058000002c800006c18000000080000000"),
        SigType.ED25519,
        "__unused__")
    wrong_length_account = Account(
        bytes.fromhex("048000002c800006c180000000800000"),
        SigType.ED25519,
        "__unused__")
    too_much_index_account = Account(
        bytes.fromhex("0b8000002c800006c1800000008000000080000000800000008000000080000000800000008000000080000000"),
        SigType.ED25519,
        "__unused__")

    for account in [wrong_number_index_account,
                    wrong_length_account,
                    too_much_index_account]:
        for sender in [lambda account: app.backend.get_public_key(account, with_prompt=False),
                       lambda account: app.backend.get_public_key(account, with_prompt=True),
                       lambda account: app.backend._ask_sign(Ins.SIGN, account),
                       lambda account: app.backend._ask_sign(Ins.SIGN_WITH_HASH, account)]:

            app.assert_screen(Screen.HOME)

            with app.expect_apdu_failure(StatusCode.WRONG_LENGTH_FOR_INS):
                sender(account)

    app.quit()

def test_wrong_class(app: TezosAppScreen):
    """Check wrong apdu class behaviour"""
    app.assert_screen(Screen.HOME)

    raw = \
        int(0x00).to_bytes(1, 'big') + \
        int(Ins.VERSION).to_bytes(1, 'big') + \
        int(Index.FIRST).to_bytes(1, 'big') + \
        int(SigType.ED25519).to_bytes(1, 'big') + \
        int(0x00).to_bytes(1, 'big')

    with app.expect_apdu_failure(StatusCode.CLASS):
        app.backend.exchange_raw(raw)

    app.assert_screen(Screen.HOME)

    raw = \
        int(0x81).to_bytes(1, 'big') + \
        int(Ins.VERSION).to_bytes(1, 'big') + \
        int(Index.FIRST).to_bytes(1, 'big') + \
        int(SigType.ED25519).to_bytes(1, 'big') + \
        int(0x00).to_bytes(1, 'big')

    with app.expect_apdu_failure(StatusCode.CLASS):
        app.backend.exchange_raw(raw)

    app.quit()

def test_wrong_apdu_length(app: TezosAppScreen):
    """Check wrong apdu length behaviour"""
    app.assert_screen(Screen.HOME)

    raw = \
        int(Cla.DEFAULT).to_bytes(1, 'big') + \
        int(Ins.VERSION).to_bytes(1, 'big') + \
        int(Index.FIRST).to_bytes(1, 'big') + \
        int(SigType.ED25519).to_bytes(1, 'big') + \
        int(0x00).to_bytes(1, 'big') + \
        int(0x00).to_bytes(1, 'big') # right size = 0x01

    with app.expect_apdu_failure(StatusCode.WRONG_LENGTH_FOR_INS):
        app.backend.exchange_raw(raw)

    app.assert_screen(Screen.HOME)

    raw = \
        int(Cla.DEFAULT).to_bytes(1, 'big') + \
        int(Ins.VERSION).to_bytes(1, 'big') + \
        int(Index.FIRST).to_bytes(1, 'big') + \
        int(SigType.ED25519).to_bytes(1, 'big') + \
        int(0x01).to_bytes(1, 'big') # right size = 0x00

    with app.expect_apdu_failure(StatusCode.WRONG_LENGTH_FOR_INS):
        app.backend.exchange_raw(raw)

    app.quit()

def test_unimplemented_commands(app: TezosAppScreen):
    """Check unimplemented commands"""
    for ins in \
        [Ins.AUTHORIZE_BAKING, \
         Ins.SIGN_UNSAFE, \
         Ins.RESET, \
         Ins.QUERY_AUTH_KEY, \
         Ins.QUERY_MAIN_HWM, \
         Ins.SETUP, \
         Ins.QUERY_ALL_HWM, \
         Ins.DEAUTHORIZE, \
         Ins.QUERY_AUTH_KEY_WITH_CURVE, \
         Ins.HMAC, \
         0xff]:

        app.assert_screen(Screen.HOME)

        with app.expect_apdu_failure(StatusCode.INVALID_INS):
            app.backend._exchange(ins)

    app.quit()
