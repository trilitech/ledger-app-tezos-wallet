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

from utils.app import Screen, DEFAULT_ACCOUNT
from utils.backend import INS, StatusCode

def test_mixing_command(app):

    app.assert_screen(Screen.Home)

    app.backend._ask_sign(INS.SIGN, DEFAULT_ACCOUNT)
    with app.expect_apdu_failure(StatusCode.UNEXPECTED_STATE):
        app.backend.version()

    app.assert_screen(Screen.Home)

    app.backend._ask_sign(INS.SIGN_WITH_HASH, DEFAULT_ACCOUNT)
    with app.expect_apdu_failure(StatusCode.UNEXPECTED_STATE):
        app.backend._ask_sign(INS.SIGN, DEFAULT_ACCOUNT)

    app.assert_screen(Screen.Home)

    app.backend._ask_sign(INS.SIGN, DEFAULT_ACCOUNT)
    with app.expect_apdu_failure(StatusCode.UNEXPECTED_STATE):
        app.backend._ask_sign(INS.SIGN_WITH_HASH, DEFAULT_ACCOUNT)

    app.assert_screen(Screen.Home)

    app.backend._ask_sign(INS.SIGN, DEFAULT_ACCOUNT)
    with app.expect_apdu_failure(StatusCode.UNEXPECTED_STATE):
        app.backend.get_public_key(DEFAULT_ACCOUNT, with_prompt=True)

    app.assert_screen(Screen.Home)

    app.backend._ask_sign(INS.SIGN, DEFAULT_ACCOUNT)
    with app.expect_apdu_failure(StatusCode.UNEXPECTED_STATE):
        app.backend.get_public_key(DEFAULT_ACCOUNT, with_prompt=False)

    app.assert_screen(Screen.Home)

    app.backend._ask_sign(INS.SIGN, DEFAULT_ACCOUNT)
    with app.expect_apdu_failure(StatusCode.UNEXPECTED_STATE):
        app.backend.git()

    app.quit()
