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

from utils.account import SIGNATURE_TYPE
from utils.app import Screen
from utils.backend import CLA, INDEX, INS, StatusCode

def test_wrong_apdu_length(app):

    app.assert_screen(Screen.Home)

    raw = \
        int(CLA.DEFAULT).to_bytes(1, 'big') + \
        int(INS.VERSION).to_bytes(1, 'big') + \
        int(INDEX.FIRST).to_bytes(1, 'big') + \
        int(SIGNATURE_TYPE.ED25519).to_bytes(1, 'big') + \
        int(0x00).to_bytes(1, 'big') + \
        int(0x00).to_bytes(1, 'big') # right size = 0x01

    with app.expect_apdu_failure(StatusCode.WRONG_LENGTH_FOR_INS):
        app.backend.exchange_raw(raw)

    app.assert_screen(Screen.Home)

    raw = \
        int(CLA.DEFAULT).to_bytes(1, 'big') + \
        int(INS.VERSION).to_bytes(1, 'big') + \
        int(INDEX.FIRST).to_bytes(1, 'big') + \
        int(SIGNATURE_TYPE.ED25519).to_bytes(1, 'big') + \
        int(0x01).to_bytes(1, 'big') # right size = 0x00

    with app.expect_apdu_failure(StatusCode.WRONG_LENGTH_FOR_INS):
        app.backend.exchange_raw(raw)

    app.quit()
