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

"""Gathering of tests related to Register-global-constant operations."""

from pathlib import Path

from utils.app import TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import RegisterGlobalConstant

def test_sign_register_global_constant(app: TezosAppScreen, snapshot_dir: Path):
    """Check signing register global constant"""

    app.setup_expert_mode()

    message = RegisterGlobalConstant(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 10000,
        counter = 2,
        gas_limit = 3,
        storage_limit = 4,
        value = {'prim': 'Pair', 'args': [{'string': '1'}, {'int': 2}]}
    )

    data = app.sign(DEFAULT_ACCOUNT,
                    message,
                    with_hash=True,
                    path=snapshot_dir)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()
