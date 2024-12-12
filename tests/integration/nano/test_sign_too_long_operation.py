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

"""Check signing too long operation behaviour"""

from pathlib import Path
from typing import Callable

from utils.app import ScreenText, TezosAppScreen, DEFAULT_ACCOUNT
from utils.backend import StatusCode
from utils.message import (
    Message,
    Proposals,
    OperationGroup,
    Reveal,
    Transaction,
    Delegation,
    RegisterGlobalConstant,
    SetDepositLimit,
    ScRollupAddMessage
)

test_path = Path(Path(__file__).stem)

def _sign_too_long(app: TezosAppScreen,
                   message: Message,
                   navigate: Callable[[], None]):

    app.setup_expert_mode()
    app.setup_blindsign_on()

    data = app._sign(
        DEFAULT_ACCOUNT,
        message,
        with_hash=True,
        navigate=navigate)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()

def _sign_decodable_too_long(app: TezosAppScreen,
                             message: Message,
                             path: Path):
    """Sign a decodable too long message"""

    def navigate() -> None:
        app.navigate_until_text(ScreenText.ACCEPT_RISK, path / "clear_n_too_long_warning")
        app.navigate_until_text(ScreenText.SIGN_ACCEPT, path / "summary")

    _sign_too_long(app, message, navigate)

def _reject_too_long(
        app,
        message: Message,
        status_code: StatusCode,
        navigate: Callable[[], None]):
    """Reject a too long message"""

    app.setup_expert_mode()
    app.setup_blindsign_on()

    app._failing_signing(
        DEFAULT_ACCOUNT,
        message,
        with_hash=True,
        navigate=navigate,
        status_code=status_code)

    app.quit()


### Too long operation ###

basic_test_path = test_path / "basic"

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

def test_sign_basic_too_long_operation(app: TezosAppScreen):
    """Check sign too long operation"""
    _sign_decodable_too_long(app, BASIC_OPERATION, basic_test_path / "accept")

def test_reject_basic_too_long_operation_at_warning(app: TezosAppScreen):
    """Check reject too long operation at warning"""
    path = basic_test_path / "reject_at_too_large_warning"

    def navigate() -> None:
        app.navigate_until_text(ScreenText.SIGN_REJECT, path / "clear_n_too_long_warning")

    _reject_too_long(app, BASIC_OPERATION, StatusCode.REJECT, navigate)

def test_reject_basic_too_long_operation_at_summary(app: TezosAppScreen):
    """Check reject too long operation at summary"""
    path = basic_test_path / "reject_at_summary"

    def navigate() -> None:
        app.navigate_until_text(ScreenText.ACCEPT_RISK, path / "clear_n_too_long_warning")
        app.navigate_until_text(ScreenText.SIGN_REJECT, path / "summary")

    _reject_too_long(app, BASIC_OPERATION, StatusCode.REJECT, navigate)


### Different kind of too long operation ###

def test_sign_too_long_operation_with_only_transactions(app: TezosAppScreen):
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
    _sign_decodable_too_long(app, message, test_path / "only_transactions")

def test_sign_too_long_operation_without_fee_or_amount(app: TezosAppScreen):
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
    _sign_decodable_too_long(app, message, test_path / "without_fee_or_amount")


### Too long operation containing a too large number ###

too_large_test_path = test_path / "too_large"

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

def test_sign_too_long_operation_with_too_large(app: TezosAppScreen):
    """Check sign too long operation that will also fail the parsing"""
    path = too_large_test_path / "accept"

    def navigate() -> None:
        app.navigate_until_text(ScreenText.ACCEPT_RISK, path / "clear_n_too_large_warning")
        app.navigate_until_text(ScreenText.SIGN_ACCEPT, path / "blindsigning")

    _sign_too_long(app, OPERATION_WITH_TOO_LARGE, navigate)

def test_reject_too_long_operation_with_too_large_at_too_large_warning(app: TezosAppScreen):
    """Check reject too long operation that will also fail the parsing at too large warning"""
    path = too_large_test_path / "reject_at_too_large_warning"

    def navigate() -> None:
        app.navigate_until_text(ScreenText.SIGN_REJECT, path / "clear_n_too_large_warning")

    _reject_too_long(app, OPERATION_WITH_TOO_LARGE, StatusCode.PARSE_ERROR, navigate)

def test_reject_too_long_operation_with_too_large_at_blindsigning(app: TezosAppScreen):
    """Check reject too long operation that will also fail the parsing at blindsigning"""
    path = too_large_test_path / "reject_at_blindsigning"

    def navigate() -> None:
        app.navigate_until_text(ScreenText.ACCEPT_RISK, path / "clear_n_too_large_warning")
        app.navigate_until_text(ScreenText.SIGN_REJECT, path / "blindsigning")

    _reject_too_long(app, OPERATION_WITH_TOO_LARGE, StatusCode.REJECT, navigate)
