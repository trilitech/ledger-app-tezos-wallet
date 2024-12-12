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

"""Check wrong apdu class behaviour"""

from utils.account import SigType
from utils.app import Screen, TezosAppScreen
from utils.backend import Index, Ins, StatusCode

def test_wrong_class(app: TezosAppScreen):
    """Check wrong apdu class behaviour"""
    app.assert_screen(Screen.HOME)

    raw = \
        int(0x00).to_bytes(1, 'big') + \
        int(Ins.VERSION).to_bytes(1, 'big') + \
        int(Index.FIRST).to_bytes(1, 'big') + \
        int(SigType.ED25519).to_bytes(1, 'big') + \
        int(0x00).to_bytes(1, 'big')

    with app.expect_apdu_failure(StatusCode.CLASS):
        app.backend.exchange_raw(raw)

    app.assert_screen(Screen.HOME)

    raw = \
        int(0x81).to_bytes(1, 'big') + \
        int(Ins.VERSION).to_bytes(1, 'big') + \
        int(Index.FIRST).to_bytes(1, 'big') + \
        int(SigType.ED25519).to_bytes(1, 'big') + \
        int(0x00).to_bytes(1, 'big')

    with app.expect_apdu_failure(StatusCode.CLASS):
        app.backend.exchange_raw(raw)

    app.quit()
