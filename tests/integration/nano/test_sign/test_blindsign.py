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

"""Gathering of tests related to Blindsign."""

from pathlib import Path
from typing import Callable, List, Union

from ragger.firmware import Firmware
from ragger.navigator import NavIns, NavInsID

from utils.account import Account
from utils.backend import TezosBackend, StatusCode
from utils.message import (
    Message,
    MichelineExpr,
    Proposals,
    OperationGroup,
    Reveal,
    Transaction,
    Delegation,
    RegisterGlobalConstant,
    SetDepositLimit,
    ScRollupAddMessage
)
from utils.navigator import ScreenText, TezosNavigator


def _sign_too_long(tezos_navigator: TezosNavigator,
                   account: Account,
                   message: Message,
                   navigate: Callable[[], None]):

    tezos_navigator.toggle_expert_mode()
    tezos_navigator.toggle_blindsign()

    data = tezos_navigator.sign(
        account,
        message,
        with_hash=True,
        navigate=navigate)

    account.check_signature(
        message=message,
        with_hash=True,
        data=data)

def _sign_decodable_too_long(tezos_navigator: TezosNavigator,
                             account: Account,
                             message: Message,
                             path: Path):
    """Sign a decodable too long message"""

    def navigate() -> None:
        tezos_navigator.navigate_review(
            text=ScreenText.ACCEPT_RISK,
            snap_path=path / "clear_n_too_long_warning"
        )
        tezos_navigator.navigate_sign_accept(snap_path=path / "summary")

    _sign_too_long(tezos_navigator, account, message, navigate)

def _reject_too_long(
        tezos_navigator: TezosNavigator,
        account: Account,
        message: Message,
        status_code: StatusCode,
        navigate: Callable[[], None]):
    """Reject a too long message"""

    tezos_navigator.toggle_expert_mode()
    tezos_navigator.toggle_blindsign()

    with status_code.expected():
        tezos_navigator.sign(
            account,
            message,
            with_hash=True,
            navigate=navigate
        )


### Too long operation ###

BASIC_OPERATION = OperationGroup([
    Reveal(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 1000000,
        counter = 11,
        gas_limit = 1,
        storage_limit = 4,
        public_key = 'p2pk66m3NQsd4n6LJWe9WMwx9WHeXwKmBaMwXX92WkMQCR99zmwk2PM'
    ),
    Transaction(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 2000000,
        counter = 12,
        gas_limit = 1,
        storage_limit = 7,
        destination = 'tz3XMQscBFM9vPmpbYMavMmwxRMUWvWGZMQQ',
        amount = 3000000,
        entrypoint = 'update_config',
        parameter = {'prim': 'Pair', 'args': [ {'int': 5}, {'prim': 'True'} ]}
    ),
    Delegation(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 3000000,
        counter = 13,
        gas_limit = 1,
        storage_limit = 5,
        delegate = 'tz2W3Tvcm64GjcV2bipUynnEsctLFz5Z6yRa'
    ),
    ScRollupAddMessage(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 4000000,
        counter = 14,
        gas_limit = 1,
        storage_limit = 6,
        message = [
            bytes.fromhex('6d65737361676530'),
            bytes.fromhex('6d65737361676531'),
            bytes.fromhex('6d65737361676532'),
            bytes.fromhex('6d65737361676533'),
            bytes.fromhex('6d65737361676534'),
            bytes.fromhex('6d65737361676535'),
            bytes.fromhex('6d65737361676536'),
            bytes.fromhex('6d65737361676537'),
            bytes.fromhex('6d65737361676538'),
            bytes.fromhex('6d65737361676539'),
            bytes.fromhex('6d6573736167653130'),
            bytes.fromhex('6d6573736167653131'),
            bytes.fromhex('6d6573736167653132'),
            bytes.fromhex('6d6573736167653133'),
            bytes.fromhex('6d6573736167653134'),
            bytes.fromhex('6d6573736167653135'),
            bytes.fromhex('6d6573736167653136'),
            bytes.fromhex('6d6573736167653137'),
            bytes.fromhex('6d6573736167653138'),
            bytes.fromhex('6d6573736167653139')
        ]
    ),
    SetDepositLimit(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 1000000,
        counter = 15,
        gas_limit = 1,
        storage_limit = 3,
        limit = 10000000
    )
])

def test_sign_basic_too_long_operation(tezos_navigator: TezosNavigator, account: Account, snapshot_dir: Path):
    """Check sign too long operation"""
    _sign_decodable_too_long(tezos_navigator, account, BASIC_OPERATION, snapshot_dir)

def test_reject_basic_too_long_operation_at_warning(tezos_navigator: TezosNavigator, account: Account, snapshot_dir: Path):
    """Check reject too long operation at warning"""

    def navigate() -> None:
        tezos_navigator.navigate_sign_reject(
            snap_path=snapshot_dir / "clear_n_too_long_warning"
        )

    _reject_too_long(tezos_navigator, account, BASIC_OPERATION, StatusCode.REJECT, navigate)

def test_reject_basic_too_long_operation_at_summary(tezos_navigator: TezosNavigator, account: Account, snapshot_dir: Path):
    """Check reject too long operation at summary"""

    def navigate() -> None:
        tezos_navigator.navigate_review(
            text=ScreenText.ACCEPT_RISK,
            snap_path=snapshot_dir / "clear_n_too_long_warning"
        )
        tezos_navigator.navigate_sign_reject(
            snap_path=snapshot_dir / "summary"
        )

    _reject_too_long(tezos_navigator, account, BASIC_OPERATION, StatusCode.REJECT, navigate)


### Different kind of too long operation ###

def test_sign_too_long_operation_with_only_transactions(tezos_navigator: TezosNavigator, account: Account, snapshot_dir: Path):
    """Check sign too long operation that contains only transaction"""
    message = OperationGroup([
        Transaction(
            source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
            fee = 0,
            counter = 11,
            gas_limit = 1,
            storage_limit = 0,
            destination = 'tz1er74kx433vTtpYddGsf3dDt5piBZeeHyQ',
            amount = 10000000
        ),
        Transaction(
            source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
            fee = 1000000,
            counter = 12,
            gas_limit = 1,
            storage_limit = 1,
            destination = 'tz2PPZ2WN4j92Rdx4NM7oW3HAp3x825HUyac',
            amount = 1000000
        ),
        Transaction(
            source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
            fee = 2000000,
            counter = 13,
            gas_limit = 1,
            storage_limit = 2,
            destination = 'tz1Kp8NCAN5WWwvkWkMmQQXMRe68iURmoQ8w',
            amount = 2000000
        ),
        Transaction(
            source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
            fee = 3000000,
            counter = 14,
            gas_limit = 1,
            storage_limit = 3,
            destination = 'tz3fLwHKthqhTPK6Lar6CTXN1WbDETw1YpGB',
            amount = 3000000
        ),
        Transaction(
            source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
            fee = 4000000,
            counter = 15,
            gas_limit = 1,
            storage_limit = 4,
            destination = 'tz3eydffbLkjdVb8zx42BvxpGV87zaRnqL3r',
            amount = 4000000
        ),
        Transaction(
            source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
            fee = 5000000,
            counter = 16,
            gas_limit = 1,
            storage_limit = 5,
            destination = 'tz2JPgTWZZpxZZLqHMfS69UAy1UHm4Aw5iHu',
            amount = 5000000
        )
    ])
    _sign_decodable_too_long(tezos_navigator, account, message, snapshot_dir)

def test_sign_too_long_operation_without_fee_or_amount(tezos_navigator: TezosNavigator, account: Account, snapshot_dir: Path):
    """Check sign too long operation that doesn't have fees or amount"""
    message = Proposals(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        proposals = [
            'ProtoDemoNoopsDemoNoopsDemoNoopsDemoNoopsDemo6XBoYp',
            'ProtoGenesisGenesisGenesisGenesisGenesisGenesk612im',
            'PrihK96nBAFSxVL1GLJTVhu9YnzkMFiBeuJRPA8NwuZVZCE1L6i',
            'Ps9mPmXaRzmzk35gbAYNCAw6UXdE2qoABTHbN2oEEc1qM7CwT9P',
            'PsBabyM1eUXZseaJdmXFApDSBqj8YBfwELoxZHHW77EMcAbbwAS',
            'PsCARTHAGazKbHtnKfLzQg3kms52kSRpgnDY982a9oYsSXRLQEb',
            'PsDELPH1Kxsxt8f9eWbxQeRxkjfbxoqM52jvs5Y5fBxWWh4ifpo',
            'PtEdo2ZkT9oKpimTah6x2embF25oss54njMuPzkJTEi5RqfdZFA',
            'PsFLorenaUUuikDWvMDr6fGBRG8kt3e3D3fHoXK1j1BFRxeSH4i',
            'PtGRANADsDU8R9daYKAgWnQYAJ64omN1o3KMGVCykShA97vQbvV',
            'PtHangz2aRngywmSRGGvrcTyMbbdpWdpFKuS4uMWxg2RaH9i1qx',
            'Psithaca2MLRFYargivpo7YvUr7wUDqyxrdhC5CQq78mRvimz6A',
            'PtJakart2xVj7pYXJBXrqHgd82rdkLey5ZeeGwDgPp9rhQUbSqY',
            'PtKathmankSpLLDALzWw7CGD2j2MtyveTwboEYokqUCP4a1LxMg',
            'PtLimaPtLMwfNinJi9rCfDPWea8dFgTZ1MeJ9f1m2SRic6ayiwW',
            'PtMumbai2TmsJHNGRkD8v8YDbtao7BLUC3wjASn1inAKLFCjaH1',
            'PtNairobiyssHuh87hEhfVBGCVrK3WnS8Z2FT4ymB5tAa4r1nQf',
            'ProxfordYmVfjWnRcgjWH36fW6PArwqykTFzotUxRs6gmTcZDuH',
            'PtParisBxoLz5gzMmn3d9WBQNoPSZakgnkMC2VNuQ3KXfUtUQeZ',
            'ProtoALphaALphaALphaALphaALphaALphaALphaALphaDdp3zK'
        ],
        period = 32
    )
    _sign_decodable_too_long(tezos_navigator, account, message, snapshot_dir)


### Too long operation containing a too large number ###

OPERATION_WITH_TOO_LARGE = OperationGroup([
    ScRollupAddMessage(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 4000000,
        counter = 11,
        gas_limit = 1,
        storage_limit = 6,
        message = [
            bytes.fromhex('6d65737361676530'),
            bytes.fromhex('6d65737361676531'),
            bytes.fromhex('6d65737361676532'),
            bytes.fromhex('6d65737361676533'),
            bytes.fromhex('6d65737361676534'),
            bytes.fromhex('6d65737361676535'),
            bytes.fromhex('6d65737361676536'),
            bytes.fromhex('6d65737361676537'),
            bytes.fromhex('6d65737361676538'),
            bytes.fromhex('6d65737361676539'),
            bytes.fromhex('6d6573736167653130'),
            bytes.fromhex('6d6573736167653131'),
            bytes.fromhex('6d6573736167653132'),
            bytes.fromhex('6d6573736167653133'),
            bytes.fromhex('6d6573736167653134'),
            bytes.fromhex('6d6573736167653135'),
            bytes.fromhex('6d6573736167653136'),
            bytes.fromhex('6d6573736167653137'),
            bytes.fromhex('6d6573736167653138'),
            bytes.fromhex('6d6573736167653139'),
            bytes.fromhex('6d6573736167653230'),
            bytes.fromhex('6d6573736167653231'),
            bytes.fromhex('6d6573736167653232'),
            bytes.fromhex('6d6573736167653233'),
            bytes.fromhex('6d6573736167653234'),
            bytes.fromhex('6d6573736167653235'),
            bytes.fromhex('6d6573736167653236'),
            bytes.fromhex('6d6573736167653237'),
            bytes.fromhex('6d6573736167653238'),
            bytes.fromhex('6d6573736167653239')
        ]
    ),
    RegisterGlobalConstant(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 5000000,
        counter = 12,
        gas_limit = 1,
        storage_limit = 3,
        value = {'int': 115792089237316195423570985008687907853269984665640564039457584007913129639936}
    )
])

def test_sign_too_long_operation_with_too_large(tezos_navigator: TezosNavigator, account: Account, snapshot_dir: Path):
    """Check sign too long operation that will also fail the parsing"""

    def navigate() -> None:
        tezos_navigator.navigate_review(
            text=ScreenText.ACCEPT_RISK,
            snap_path=snapshot_dir / "clear_n_too_large_warning"
        )
        tezos_navigator.navigate_sign_accept(
            snap_path=snapshot_dir / "blindsigning"
        )

    _sign_too_long(tezos_navigator, account, OPERATION_WITH_TOO_LARGE, navigate)

def test_reject_too_long_operation_with_too_large_at_too_large_warning(tezos_navigator: TezosNavigator, account: Account, snapshot_dir: Path):
    """Check reject too long operation that will also fail the parsing at too large warning"""

    def navigate() -> None:
        tezos_navigator.navigate_sign_reject(
            snap_path=snapshot_dir / "clear_n_too_large_warning"
        )

    _reject_too_long(tezos_navigator, account, OPERATION_WITH_TOO_LARGE, StatusCode.PARSE_ERROR, navigate)

def test_reject_too_long_operation_with_too_large_at_blindsigning(tezos_navigator: TezosNavigator, account: Account, snapshot_dir: Path):
    """Check reject too long operation that will also fail the parsing at blindsigning"""

    def navigate() -> None:
        tezos_navigator.navigate_review(
            text=ScreenText.ACCEPT_RISK,
            snap_path=snapshot_dir / "clear_n_too_large_warning"
        )
        tezos_navigator.navigate_sign_reject(
            snap_path=snapshot_dir / "blindsigning"
        )

    _reject_too_long(tezos_navigator, account, OPERATION_WITH_TOO_LARGE, StatusCode.REJECT, navigate)

def test_blindsign_too_deep(
        backend: TezosBackend,
        firmware: Firmware,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path):
    """Check blindsigning on too deep expression"""

    expression = MichelineExpr([[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[{'int':42}]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]])

    def navigate() -> None:
        if firmware == Firmware.NANOS:
            ### Simulate `navigate_review` up to `ACCEPT_RISK` because the nanos screen can look like it hasn't changed.

            instructions: List[Union[NavIns, NavInsID]] = [
                # 'Review operation'
                NavInsID.RIGHT_CLICK,  # 'Expression {{{...{{{'
                NavInsID.RIGHT_CLICK,  # 'Expression {{{...{{{'
                NavInsID.RIGHT_CLICK,  # 'The transaction cannot be trusted.'
                NavInsID.RIGHT_CLICK,  # 'Parsing error ERR_TOO_DEEP'
                NavInsID.RIGHT_CLICK,  # 'Learn More: bit.ly/ledger-tez'
                NavInsID.RIGHT_CLICK,  # 'Accept risk'
                NavInsID.BOTH_CLICK,
            ]

            tezos_navigator.unsafe_navigate(
                instructions=instructions,
                screen_change_before_first_instruction=True,
                screen_change_after_last_instruction=False,
                snap_path=snapshot_dir / "clear",
            )
        else:
            tezos_navigator.navigate_review(
                text=ScreenText.ACCEPT_RISK,
                snap_path=snapshot_dir / "clear"
            )

        tezos_navigator.navigate_sign_accept(
            snap_path=snapshot_dir / "blind"
        )

    data = tezos_navigator.sign(
        account,
        expression,
        with_hash=True,
        navigate=navigate
    )

    account.check_signature(
        message=expression,
        with_hash=True,
        data=data)

def test_blindsign_too_large(tezos_navigator: TezosNavigator, account: Account, snapshot_dir: Path):
    """Check blindsigning on too large expression"""

    message = MichelineExpr({'int':12345678901234567890123456789012345678901234567890123456789012345678901234567890})

    def navigate() -> None:
        tezos_navigator.navigate_review(
            text=ScreenText.ACCEPT_RISK,
            snap_path=snapshot_dir / "clear"
        )
        tezos_navigator.navigate_sign_accept(
            snap_path=snapshot_dir / "blind"
        )

    data = tezos_navigator.sign(
        account,
        message,
        with_hash=True,
        navigate=navigate
    )

    account.check_signature(
        message=message,
        with_hash=True,
        data=data)

def test_blindsign_reject_from_clear(tezos_navigator: TezosNavigator, account: Account, snapshot_dir: Path):
    """Check blindsigning rejection"""

    expression = MichelineExpr({'int':12345678901234567890123456789012345678901234567890123456789012345678901234567890})

    with StatusCode.PARSE_ERROR.expected():
        tezos_navigator.reject_signing(
            account,
            expression,
            with_hash=False,
            snap_path=snapshot_dir
        )

def test_blindsign_reject_from_blind(tezos_navigator: TezosNavigator, account: Account, snapshot_dir: Path):
    """Check blindsigning rejection"""

    expression = MichelineExpr({'int':12345678901234567890123456789012345678901234567890123456789012345678901234567890})

    def navigate() -> None:
        tezos_navigator.navigate_review(
            text=ScreenText.ACCEPT_RISK,
            snap_path=snapshot_dir / "clear"
        )
        tezos_navigator.navigate_sign_reject(
            snap_path=snapshot_dir / "blind"
        )

    with StatusCode.REJECT.expected():
        tezos_navigator.sign(
            account,
            expression,
            with_hash=False,
            navigate=navigate
        )

def test_ensure_always_clearsign(
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path):
    """Check clear signing never blindsign"""

    tezos_navigator.toggle_expert_mode()

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

    data = tezos_navigator.sign(
        account,
        message,
        with_hash=True,
        snap_path=snapshot_dir
    )

    account.check_signature(
        message=message,
        with_hash=True,
        data=data)