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

"""Gathering of tests related to Parsing error."""

from pathlib import Path

import pytest

from utils.account import Account
from utils.backend import StatusCode, TezosBackend
from utils.message import RawMessage
from utils.navigator import TezosNavigator


# Operation (0): Transaction
# Source: tz2JPgTWZZpxZZLqHMfS69UAy1UHm4Aw5iHu
# Fee: 0.05 XTZ
# Gas limit: 54
# Counter: 8
# Storage limit: 45
# Amount: 0.24 XTZ
# Destination: KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT
# Entrypoint: do
# Parameter: CAR

# original bytes : 0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316

@pytest.mark.parametrize(
    "raw_msg", [
        "0100000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316",
        "03000000000000000000000000000000000000000000000000000000000000000001016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316",
        "0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e010000000000000000000000000000000000000000ff02000000020316",
        "0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff0200000002031645",
        "0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e0100000000000000000000000000000000000000000000ff02000000020316",
    ],
    ids=[
        "unknown_magic_bytes",
        "unknown_operation",
        "one_byte_removed_inside",
        "one_byte_added_at_the_end",
        "one_byte_added_inside",
    ]
)
def test_parsing_error(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        raw_msg: str,
        account: Account,
        snapshot_dir: Path
):
    """Check parsing error handling"""

    tezos_navigator.toggle_expert_mode()

    with StatusCode.PARSE_ERROR.expected():
        with backend.sign(
                account,
                RawMessage(raw_msg),
                with_hash=True
        ):
            tezos_navigator.refuse_sign_error_risk(snap_path=snapshot_dir)

@pytest.mark.parametrize(
    "raw_msg", [
        "030000000000000000000000000000000000000000000000000000000000000000ce00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e02030400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000c63966303966323935326433343532386337333366393436313563666333396263353535363139666335353064643461363762613232303863653865383637616133643133613665663939646662653332633639373461613961323135306432316563613239633333343965353963313362393038316631",
    ],
    ids=[
        "wrong_last_packet",
    ]
)
def test_parsing_hard_fail(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        raw_msg: str,
        account: Account,
        snapshot_dir: Path
):
    """Check parsing error hard failing"""

    tezos_navigator.toggle_expert_mode()

    with StatusCode.UNEXPECTED_SIGN_STATE.expected():
        with backend.sign(
                account,
                RawMessage(raw_msg),
                with_hash=True
        ):
            tezos_navigator.hard_reject_sign(snap_path=snapshot_dir)
