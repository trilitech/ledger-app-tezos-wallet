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

"""Gathering of tests related to Failing-noop operations."""

from pathlib import Path

from utils.app import Screen, TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import FailingNoop

def test_sign_failing_noop(app: TezosAppScreen, snapshot_dir: Path):
    """Check signing failing noop"""

    app.assert_screen(Screen.HOME)

    message = FailingNoop("9f09f2952d34528c733f94615cfc39bc555619fc550dd4a67ba2208ce8e867aa3d13a6ef99dfbe32c6974aa9a2150d21eca29c3349e59c13b9081f1c11b440ac4d3455dedbe4ee0de15a8af620d4c86247d9d132de1bb6da23d5ff9d8dffda22ba9a84")

    with app.backend.sign(DEFAULT_ACCOUNT, message, with_hash=True) as result:
        app.accept_sign(snap_path=snapshot_dir)

    DEFAULT_ACCOUNT.check_signature(
        message=message,
        with_hash=True,
        data=result.value
    )

    app.quit()
