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

"""Gathering of tests related to app menu."""

from pathlib import Path
from typing import List, Union

import requests

from ragger.navigator import NavIns, NavInsID

from utils.backend import TezosBackend
from utils.navigator import TezosNavigator


def test_home_menu(tezos_navigator: TezosNavigator, snapshot_dir: Path):
    """Check home menu flow"""
    instructions: List[Union[NavIns, NavInsID]] = [
        # Home
        NavInsID.RIGHT_CLICK,  # Version
        NavInsID.RIGHT_CLICK,  # Settings
        NavInsID.RIGHT_CLICK,  # Quit
    ]
    tezos_navigator.navigate(
        instructions=instructions,
        snap_path=snapshot_dir,
    )


def test_settings_menu(tezos_navigator: TezosNavigator, snapshot_dir: Path):
    """Check settings menu flow"""
    tezos_navigator.navigate_to_settings()
    instructions: List[Union[NavIns, NavInsID]] = [
        # Expert Mode
        NavInsID.RIGHT_CLICK,  # Blind Sign
        NavInsID.RIGHT_CLICK,  # Back
        NavInsID.BOTH_CLICK,  # Home
    ]
    tezos_navigator.navigate(
        instructions=instructions,
        snap_path=snapshot_dir
    )


def test_toggle_expert_mode(tezos_navigator: TezosNavigator, snapshot_dir: Path):
    """Check settings' expert_mode toggle"""
    snap_idx = tezos_navigator.toggle_expert_mode(snap_path=snapshot_dir)
    # Toggle back
    tezos_navigator.toggle_expert_mode(snap_start_idx=snap_idx, snap_path=snapshot_dir)


def test_toggle_blindsign(tezos_navigator: TezosNavigator, snapshot_dir: Path):
    """Check settings' blindsign toggle"""
    snap_idx = tezos_navigator.toggle_blindsign(snap_path=snapshot_dir)
    # Toggle back
    tezos_navigator.toggle_blindsign(snap_start_idx=snap_idx, snap_path=snapshot_dir)


def test_quit(backend: TezosBackend):
    """Check quit app"""
    # Home
    backend.left_click()
    backend.wait_for_screen_change()  # Quit
    try:
        backend.both_click()
        assert False, "Must have lost connection with speculos"
    except requests.exceptions.ConnectionError:
        pass
