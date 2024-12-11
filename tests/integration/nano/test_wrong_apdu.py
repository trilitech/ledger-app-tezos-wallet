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
from typing import Any, Callable, Union

import pytest

from utils.account import Account, SigType
from utils.app import TezosAppScreen
from utils.backend import Cla, Index, Ins, StatusCode
from utils.message import Transaction

def test_regression_continue_after_reject(app: TezosAppScreen, account: Account, snapshot_dir: Path):
    """Check the app still runs after rejects signing"""

    app.toggle_expert_mode()

    with StatusCode.REJECT.expected():
        with app.backend.prompt_public_key(account):
            app.reject_public_key(snap_path=snapshot_dir / "reject_public_key")

    app.backend.wait_for_home_screen()

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
        with app.backend.sign(account, message, with_hash=True):
            app.reject_sign(snap_path=snapshot_dir / "reject_signing")

    app.backend.get_public_key(account)


def test_change_sign_instruction(app: TezosAppScreen, account: Account):
    """Check signing instruction changes behaviour"""

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

    app.backend._ask_sign(Ins.SIGN_WITH_HASH, account)

    with StatusCode.INVALID_INS.expected():
        app.backend._continue_sign(Ins.SIGN,
                                   payload,
                                   last=True)

    app.backend._ask_sign(Ins.SIGN, account)

    with StatusCode.INVALID_INS.expected():
        app.backend._continue_sign(Ins.SIGN_WITH_HASH,
                                   payload,
                                   last=True)

def test_mixing_command(app: TezosAppScreen, account: Account):
    """Check that mixing instruction fails"""

    app.backend._ask_sign(Ins.SIGN, account)
    with StatusCode.UNEXPECTED_STATE.expected():
        app.backend.version()

    app.backend._ask_sign(Ins.SIGN_WITH_HASH, account)
    with StatusCode.UNEXPECTED_STATE.expected():
        app.backend._ask_sign(Ins.SIGN, account)

    app.backend._ask_sign(Ins.SIGN, account)
    with StatusCode.UNEXPECTED_STATE.expected():
        app.backend._ask_sign(Ins.SIGN_WITH_HASH, account)

    app.backend._ask_sign(Ins.SIGN, account)
    with StatusCode.UNEXPECTED_STATE.expected():
        with app.backend.prompt_public_key(account):
            pass

    app.backend._ask_sign(Ins.SIGN, account)
    with StatusCode.UNEXPECTED_STATE.expected():
        app.backend.get_public_key(account)

    app.backend._ask_sign(Ins.SIGN, account)
    with StatusCode.UNEXPECTED_STATE.expected():
        app.backend.git()

@pytest.mark.parametrize("ins", [Ins.GET_PUBLIC_KEY, Ins.PROMPT_PUBLIC_KEY], ids=lambda ins: f"{ins}")
@pytest.mark.parametrize("index", [Index.OTHER, Index.LAST], ids=lambda index: f"{index}")
def test_wrong_index(app: TezosAppScreen, account: Account, ins: Ins, index: Index):
    """Check wrong apdu index behaviour"""

    with StatusCode.WRONG_PARAM.expected():
        app.backend._exchange(ins,
                              index=index,
                              sig_type=account.sig_type,
                              payload=account.path)


@pytest.mark.parametrize(
    "sender",
    [
        lambda app, account: app.backend._provide_public_key(account, with_prompt=False),
        lambda app, account: app.backend._provide_public_key(account, with_prompt=True),
        lambda app, account: app.backend._ask_sign(Ins.SIGN, account),
        lambda app, account: app.backend._ask_sign(Ins.SIGN_WITH_HASH, account)
    ],
    ids=[
        "get_pk_without_prompt",
        "get_pk_with_prompt",
        "sign_without_hash",
        "sign_with_hash",
    ]
)
def test_wrong_derivation_type(app: TezosAppScreen, sender: Callable[[TezosAppScreen, Account], Any]):
    """Check wrong derivation type behaviour"""
    account = Account("m/44'/1729'/0'/0'", 0x04, "__unused__")

    with StatusCode.WRONG_PARAM.expected():
        sender(app, account)


@pytest.mark.parametrize(
    "sender",
    [
        lambda app, account: app.backend._provide_public_key(account, with_prompt=False),
        lambda app, account: app.backend._provide_public_key(account, with_prompt=True),
        lambda app, account: app.backend._ask_sign(Ins.SIGN, account),
        lambda app, account: app.backend._ask_sign(Ins.SIGN_WITH_HASH, account)
    ],
    ids=[
        "get_pk_without_prompt",
        "get_pk_with_prompt",
        "sign_without_hash",
        "sign_with_hash",
    ]
)
@pytest.mark.parametrize(
    "account",
    [
        Account(
            bytes.fromhex("058000002c800006c18000000080000000"),
            SigType.ED25519,
            "__unused__"),
        Account(
            bytes.fromhex("048000002c800006c180000000800000"),
            SigType.ED25519,
            "__unused__"),
        Account(
            bytes.fromhex("0b8000002c800006c1800000008000000080000000800000008000000080000000800000008000000080000000"),
            SigType.ED25519,
            "__unused__"),
    ],
    ids=[
        "wrong_number_index_account",
        "wrong_length_account",
        "too_much_index_account",
    ]
)
def test_wrong_derivation_path(
        app: TezosAppScreen,
        account: Account,
        sender: Callable[[TezosAppScreen, Account], Any]):
    """Check wrong derivation path behaviour"""

    with StatusCode.WRONG_LENGTH_FOR_INS.expected():
        sender(app, account)

@pytest.mark.parametrize("class_", [0x00, 0x81])
def test_wrong_class(app: TezosAppScreen, class_: int):
    """Check wrong apdu class behaviour"""

    raw = \
        class_.to_bytes(1, 'big') + \
        int(Ins.VERSION).to_bytes(1, 'big') + \
        int(Index.FIRST).to_bytes(1, 'big') + \
        int(SigType.ED25519).to_bytes(1, 'big') + \
        int(0x00).to_bytes(1, 'big')

    with StatusCode.CLASS.expected():
        app.backend.exchange_raw(raw)

@pytest.mark.parametrize(
    "size, data",
    [
        (0, b'\x00'),
        (1, b'')
    ],
    ids=lambda param: f"size={param}" if isinstance(param, int) else f"data={param}"
)
def test_wrong_apdu_length(app: TezosAppScreen, size: int, data: bytes):
    """Check wrong apdu length behaviour"""

    raw = \
        int(Cla.DEFAULT).to_bytes(1, 'big') + \
        int(Ins.VERSION).to_bytes(1, 'big') + \
        int(Index.FIRST).to_bytes(1, 'big') + \
        int(SigType.ED25519).to_bytes(1, 'big') + \
        size.to_bytes(1, 'big') + \
        data

    with StatusCode.WRONG_LENGTH_FOR_INS.expected():
        app.backend.exchange_raw(raw)

@pytest.mark.parametrize(
    "ins",
    [
        Ins.AUTHORIZE_BAKING,
        Ins.SIGN_UNSAFE,
        Ins.RESET,
        Ins.QUERY_AUTH_KEY,
        Ins.QUERY_MAIN_HWM,
        Ins.SETUP,
        Ins.QUERY_ALL_HWM,
        Ins.DEAUTHORIZE,
        Ins.QUERY_AUTH_KEY_WITH_CURVE,
        Ins.HMAC,
        0xff
    ],
    ids=lambda ins: f"ins={ins}"
)
def test_unimplemented_commands(app: TezosAppScreen, ins: Union[int, Ins]):
    """Check unimplemented commands"""

    with StatusCode.INVALID_INS.expected():
        app.backend._exchange(ins)
