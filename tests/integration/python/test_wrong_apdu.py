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

from typing import Any, Callable, Union

import pytest

from utils.account import Account, SigType
from utils.backend import Cla, TezosBackend, Index, Ins, StatusCode
from utils.message import Transaction
from utils.navigator import TezosNavigator


def test_regression_continue_after_reject(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account
):
    """Check the app still runs after rejects signing"""

    tezos_navigator.toggle_expert_mode()

    with StatusCode.REJECT.expected():
        with backend.prompt_public_key(account):
            tezos_navigator.reject_public_key()

    backend.wait_for_home_screen()

    message = Transaction()

    with StatusCode.REJECT.expected():
        with backend.sign(account, message, with_hash=True):
            tezos_navigator.reject_sign()

    backend.get_public_key(account)


def test_change_sign_instruction(backend: TezosBackend, account: Account):
    """Check signing instruction changes behaviour"""

    message = Transaction()
    payload = bytes(message)

    backend._ask_sign(Ins.SIGN_WITH_HASH, account)

    with StatusCode.INVALID_INS.expected():
        backend._continue_sign(
            Ins.SIGN,
            payload,
            last=True)

    backend._ask_sign(Ins.SIGN, account)

    with StatusCode.INVALID_INS.expected():
        backend._continue_sign(
            Ins.SIGN_WITH_HASH,
            payload,
            last=True)

def test_mixing_command(backend: TezosBackend, account: Account):
    """Check that mixing instruction fails"""

    backend._ask_sign(Ins.SIGN, account)
    with StatusCode.UNEXPECTED_STATE.expected():
        backend.version()

    backend._ask_sign(Ins.SIGN_WITH_HASH, account)
    with StatusCode.UNEXPECTED_STATE.expected():
        backend._ask_sign(Ins.SIGN, account)

    backend._ask_sign(Ins.SIGN, account)
    with StatusCode.UNEXPECTED_STATE.expected():
        backend._ask_sign(Ins.SIGN_WITH_HASH, account)

    backend._ask_sign(Ins.SIGN, account)
    with StatusCode.UNEXPECTED_STATE.expected():
        with backend.prompt_public_key(account):
            pass

    backend._ask_sign(Ins.SIGN, account)
    with StatusCode.UNEXPECTED_STATE.expected():
        backend.get_public_key(account)

    backend._ask_sign(Ins.SIGN, account)
    with StatusCode.UNEXPECTED_STATE.expected():
        backend.git()

@pytest.mark.parametrize("ins", [Ins.GET_PUBLIC_KEY, Ins.PROMPT_PUBLIC_KEY], ids=lambda ins: f"{ins}")
@pytest.mark.parametrize("index", [Index.OTHER, Index.LAST], ids=lambda index: f"{index}")
def test_wrong_index(backend: TezosBackend, account: Account, ins: Ins, index: Index):
    """Check wrong apdu index behaviour"""

    with StatusCode.WRONG_PARAM.expected():
        backend._exchange(
            ins,
            index=index,
            sig_type=account.sig_type,
            payload=account.path
        )


@pytest.mark.parametrize(
    "sender",
    [
        lambda backend, account: backend._provide_public_key(account, with_prompt=False),
        lambda backend, account: backend._provide_public_key(account, with_prompt=True),
        lambda backend, account: backend._ask_sign(Ins.SIGN, account),
        lambda backend, account: backend._ask_sign(Ins.SIGN_WITH_HASH, account)
    ],
    ids=[
        "get_pk_without_prompt",
        "get_pk_with_prompt",
        "sign_without_hash",
        "sign_with_hash",
    ]
)
def test_wrong_derivation_type(backend: TezosBackend, sender: Callable[[TezosBackend, Account], Any]):
    """Check wrong derivation type behaviour"""
    account = Account("m/44'/1729'/0'/0'", 0x04, "__unused__")

    with StatusCode.WRONG_PARAM.expected():
        sender(backend, account)


@pytest.mark.parametrize(
    "sender",
    [
        lambda backend, account: backend._provide_public_key(account, with_prompt=False),
        lambda backend, account: backend._provide_public_key(account, with_prompt=True),
        lambda backend, account: backend._ask_sign(Ins.SIGN, account),
        lambda backend, account: backend._ask_sign(Ins.SIGN_WITH_HASH, account)
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
        backend: TezosBackend,
        account: Account,
        sender: Callable[[TezosBackend, Account], Any]):
    """Check wrong derivation path behaviour"""

    with StatusCode.WRONG_LENGTH_FOR_INS.expected():
        sender(backend, account)

@pytest.mark.parametrize("class_", [0x00, 0x81])
def test_wrong_class(backend: TezosBackend, class_: int):
    """Check wrong apdu class behaviour"""

    raw = \
        class_.to_bytes(1, 'big') + \
        int(Ins.VERSION).to_bytes(1, 'big') + \
        int(Index.FIRST).to_bytes(1, 'big') + \
        int(SigType.ED25519).to_bytes(1, 'big') + \
        int(0x00).to_bytes(1, 'big')

    with StatusCode.CLASS.expected():
        backend.exchange_raw(raw)

@pytest.mark.parametrize(
    "size, data",
    [
        (0, b'\x00'),
        (1, b'')
    ],
    ids=lambda param: f"size={param}" if isinstance(param, int) else f"data={param}"
)
def test_wrong_apdu_length(backend: TezosBackend, size: int, data: bytes):
    """Check wrong apdu length behaviour"""

    raw = \
        int(Cla.DEFAULT).to_bytes(1, 'big') + \
        int(Ins.VERSION).to_bytes(1, 'big') + \
        int(Index.FIRST).to_bytes(1, 'big') + \
        int(SigType.ED25519).to_bytes(1, 'big') + \
        size.to_bytes(1, 'big') + \
        data

    with StatusCode.WRONG_LENGTH_FOR_INS.expected():
        backend.exchange_raw(raw)

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
def test_unimplemented_commands(backend: TezosBackend, ins: Union[int, Ins]):
    """Check unimplemented commands"""

    with StatusCode.INVALID_INS.expected():
        backend._exchange(ins)
