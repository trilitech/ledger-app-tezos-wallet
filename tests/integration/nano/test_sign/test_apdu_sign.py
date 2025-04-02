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

"""Gathering of tests related to Sign instructions."""

from utils.account import Account
from utils.backend import TezosBackend
from utils.message import MichelineExpr, Transaction
from utils.navigator import TezosNavigator


def test_sign_micheline_without_hash(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account
):
    """Check signing micheline wihout getting hash"""

    message = MichelineExpr([{'string': 'CACA'}, {'string': 'POPO'}, {'string': 'BOUDIN'}])

    with backend.sign(account, message, with_hash=False) as result:
        tezos_navigator.accept_sign()

    account.check_signature(
        message=message,
        with_hash=False,
        data=result.value
    )

def test_sign_with_small_packet(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account
):
    """Check signing using small packet instead of full size packets"""

    tezos_navigator.toggle_expert_mode()

    message = Transaction()

    with backend.sign(account, message, apdu_size=10) as result:
        tezos_navigator.accept_sign()

    account.check_signature(
        message=message,
        with_hash=False,
        data=result.value
    )
