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

from pathlib import Path

import pytest

from ragger.navigator import NavInsID

from utils.account import Account
from utils.backend import StatusCode, TezosBackend
from utils.message import Default, RegisterGlobalConstant
from utils.navigator import TezosNavigator, TezosNavInsID


EXPERT_OPERATION = RegisterGlobalConstant(
    {"prim": "constant", 'args': [{"string": Default.SCRIPT_EXPR_HASH}]}
)


def test_reject_expert_mode(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check expert mode reject."""

    with StatusCode.REJECT.expected():
        with backend.sign(account, EXPERT_OPERATION):
            tezos_navigator.expert_reject_sign(snap_path=snapshot_dir)


@pytest.mark.use_on_device("touch")
def test_enable_expert_mode_in_review(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check that expert mode can be activated from review on touch devices."""

    with backend.sign(account, EXPERT_OPERATION):
        tezos_navigator.expert_accept_sign(snap_path=snapshot_dir / "review_expert")
        tezos_navigator.accept_sign(snap_path=snapshot_dir / "review")

    # To ensure that expert mode is ON
    tezos_navigator.navigate_to_settings(
        screen_change_before_first_instruction=True,
        snap_path=snapshot_dir / "check"
    )


@pytest.mark.use_on_device("touch")
def test_reject_sign_at_expert_mode_after_enabling(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check reject at expert splash screen after enabling expert mode"""

    with StatusCode.REJECT.expected():
        with backend.sign(account, EXPERT_OPERATION):
            tezos_navigator.expert_accept_sign(
                screen_change_after_last_instruction=False
            )
            tezos_navigator.navigate(
                instructions=[
                    NavInsID.USE_CASE_REVIEW_REJECT,
                    TezosNavInsID.REJECT_CHOICE_CONFIRM,
                    NavInsID.USE_CASE_STATUS_DISMISS,
                ],
                screen_change_before_first_instruction=True,
                snap_path=snapshot_dir
            )


@pytest.mark.use_on_device("touch")
def test_reject_sign_at_expert_mode_when_enabled(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        snapshot_dir: Path
):
    """Check reject at expert splash screen if expert mode already enabled"""

    tezos_navigator.toggle_expert_mode()

    with StatusCode.REJECT.expected():
        with backend.sign(account, EXPERT_OPERATION):
            tezos_navigator.expert_splash_navigate(
                validation_instructions=[
                    NavInsID.USE_CASE_REVIEW_REJECT,
                    TezosNavInsID.REJECT_CHOICE_CONFIRM,
                    NavInsID.USE_CASE_STATUS_DISMISS,
                ],
                snap_path=snapshot_dir
            )
