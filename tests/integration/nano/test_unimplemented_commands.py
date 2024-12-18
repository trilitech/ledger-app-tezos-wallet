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

"""Check unimplemented commands"""

from utils.app import Screen, TezosAppScreen
from utils.backend import Ins, StatusCode

def test_unimplemented_commands(app: TezosAppScreen):
    """Check unimplemented commands"""
    for ins in \
        [Ins.AUTHORIZE_BAKING, \
         Ins.SIGN_UNSAFE, \
         Ins.RESET, \
         Ins.QUERY_AUTH_KEY, \
         Ins.QUERY_MAIN_HWM, \
         Ins.SETUP, \
         Ins.QUERY_ALL_HWM, \
         Ins.DEAUTHORIZE, \
         Ins.QUERY_AUTH_KEY_WITH_CURVE, \
         Ins.HMAC, \
         0xff]:

        app.assert_screen(Screen.HOME)

        with app.expect_apdu_failure(StatusCode.INVALID_INS):
            app.backend._exchange(ins)

    app.quit()
