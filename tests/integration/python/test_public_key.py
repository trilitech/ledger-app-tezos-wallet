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

"""Gathering of tests related to public key."""

from pathlib import Path

import pytest

from utils.account import Account, PublicKey, SigType
from utils.backend import TezosBackend, StatusCode
from utils.navigator import TezosNavigator

accounts = [
    Account("m/44'/1729'/0'/0'",
            SigType.ED25519,
            "edpkuXX2VdkdXzkN11oLCb8Aurdo1BTAtQiK8ZY9UPj2YMt3AHEpcY"),
    Account("m/44'/1729'/0'/0'",
            SigType.SECP256K1,
            "sppk7bVy617DmGvXsMqcwsiLtnedTN2trUi5ugXcNig7en4rHJyunK1"),
    Account("m/44'/1729'/0'/0'",
            SigType.SECP256R1,
            "p2pk67fq5pzuMMABZ9RDrooYbLrgmnQbLt8z7PTGM9mskf7LXS5tdBG"),
    Account("m/44'/1729'/0'/0'",
            SigType.BIP32_ED25519,
            "edpkumJgSsSxkpiB5hmTq6eZcrmc6BsJtLAhYceFTiziFqje4mongz")
]

@pytest.mark.parametrize("account", accounts, ids=lambda account: f"{account.sig_type}")
def test_get_pk(backend: TezosBackend, account: Account):
    """Test that public keys get from the app are correct."""

    expected_public_key = account.key.public_key()

    data = backend.get_public_key(account)

    public_key = PublicKey.from_bytes(data, account.sig_type)

    assert public_key == expected_public_key.encode(), \
        f"Expected public key {expected_public_key} but got {public_key}"


@pytest.mark.parametrize("account", accounts, ids=lambda account: f"{account.sig_type}")
def test_provide_pk(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Test that public keys get from the app are correct and correctly displayed."""

    expected_public_key = account.key.public_key()

    with backend.prompt_public_key(account) as result:
        tezos_navigator.accept_public_key(snap_path=snapshot_dir)

    public_key = PublicKey.from_bytes(result.value, account.sig_type)

    assert public_key == expected_public_key.encode(), \
        f"Expected public key {expected_public_key} but got {public_key}"


@pytest.mark.use_on_device("touch")
def test_show_qr(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Test QR code flow."""

    with backend.prompt_public_key(account):
        tezos_navigator.accept_public_key(show_qr=True, snap_path=snapshot_dir)


def test_reject_pk(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check reject pk behaviour"""

    with StatusCode.REJECT.expected():
        with backend.prompt_public_key(account):
            tezos_navigator.reject_public_key(snap_path=snapshot_dir)
