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

"""Check signing instruction changes behaviour"""

from utils.app import Screen, TezosAppScreen, DEFAULT_ACCOUNT
from utils.backend import Ins, StatusCode
from utils.message import Transaction

def test_change_sign_instruction(app: TezosAppScreen):
    """Check signing instruction changes behaviour"""

    app.assert_screen(Screen.HOME)

    message = Transaction(
        source = 'tz2JPgTWZZpxZZLqHMfS69UAy1UHm4Aw5iHu',
        fee = 50000,
        counter = 8,
        gas_limit = 54,
        storage_limit = 45,
        destination = 'KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT',
        amount = 240000,
        entrypoint = 'do',
        parameter = {'prim': 'CAR'}
    )
    payload=bytes(message)

    app.backend._ask_sign(Ins.SIGN_WITH_HASH, DEFAULT_ACCOUNT)

    with app.expect_apdu_failure(StatusCode.INVALID_INS):
        app.backend._continue_sign(Ins.SIGN,
                                   payload,
                                   last=True)

    app.assert_screen(Screen.HOME)

    app.backend._ask_sign(Ins.SIGN, DEFAULT_ACCOUNT)

    with app.expect_apdu_failure(StatusCode.INVALID_INS):
        app.backend._continue_sign(Ins.SIGN_WITH_HASH,
                                   payload,
                                   last=True)

    app.quit()
