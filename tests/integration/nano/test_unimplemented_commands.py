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

from utils.app import Screen
from utils.backend import INS, StatusCode

def test_unimplemented_commands(app):

    for ins in \
        [INS.AUTHORIZE_BAKING, \
         INS.SIGN_UNSAFE, \
         INS.RESET, \
         INS.QUERY_AUTH_KEY, \
         INS.QUERY_MAIN_HWM, \
         INS.SETUP, \
         INS.QUERY_ALL_HWM, \
         INS.DEAUTHORIZE, \
         INS.QUERY_AUTH_KEY_WITH_CURVE, \
         INS.HMAC, \
         0xff]:

        app.assert_screen(Screen.Home)

        with app.expect_apdu_failure(StatusCode.INVALID_INS):
            app.backend._exchange(ins)

    app.quit()
