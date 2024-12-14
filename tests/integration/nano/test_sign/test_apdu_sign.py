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
from utils.navigator import send_and_navigate, TezosNavigator


def test_sign_micheline_without_hash(tezos_navigator: TezosNavigator, account: Account):
    """Check signing micheline wihout getting hash"""

    message = MichelineExpr([{'string': 'CACA'}, {'string': 'POPO'}, {'string': 'BOUDIN'}])

    data = tezos_navigator.sign(
        account,
        message,
        with_hash=False
    )

    account.check_signature(
        message=message,
        with_hash=False,
        data=data)

def test_sign_with_small_packet(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account):
    """Check signing using small packet instead of full size packets"""

    tezos_navigator.toggle_expert_mode()

    message = Transaction()

    data = send_and_navigate(
        send=lambda: backend.sign(account, message, apdu_size=10),
        navigate=tezos_navigator.navigate_sign_accept
    )

    account.check_signature(
        message=message,
        with_hash=False,
        data=data
    )
