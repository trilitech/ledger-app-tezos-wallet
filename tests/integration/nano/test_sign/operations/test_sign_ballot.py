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

"""Gathering of tests related to Ballot operations."""

from pathlib import Path

from utils.app import Screen, TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import Ballot

def test_sign_ballot(app: TezosAppScreen, snapshot_dir: Path):
    """Check signing ballot"""

    app.assert_screen(Screen.HOME)

    message = Ballot(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        proposal = 'ProtoALphaALphaALphaALphaALphaALphaALpha61322gcLUGH',
        ballot = 'yay',
        period = 32
    )

    with app.backend.sign(DEFAULT_ACCOUNT, message, with_hash=True) as result:
        app.accept_sign(snap_path=snapshot_dir)

    DEFAULT_ACCOUNT.check_signature(
        message=message,
        with_hash=True,
        data=result.value
    )

    app.quit()
