#!/usr/bin/env python3
# Copyright 2023 Functori <contact@functori.com>

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Check signing set consensus key"""

from pathlib import Path

from utils.app import Screen, TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import UpdateConsensusKey

def test_sign_set_consensus_key(app: TezosAppScreen):
    """Check signing set consensus key"""
    test_name = Path(__file__).stem

    app.assert_screen(Screen.HOME)

    message = UpdateConsensusKey(
        source = 'tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E',
        fee = 10000,
        counter = 2,
        gas_limit = 3,
        storage_limit = 4,
        pk = "edpkuXX2VdkdXzkN11oLCb8Aurdo1BTAtQiK8ZY9UPj2YMt3AHEpcY"
    )

    data = app.sign(DEFAULT_ACCOUNT,
                    message,
                    with_hash=True,
                    path=test_name)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()
