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

from utils import *

if __name__ == "__main__":
    app = stax_app()

    app.assert_screen(SCREEN_HOME_DEFAULT)

    # INS_AUTHORIZE_BAKING
    app.send_apdu("8001000000")
    app.expect_apdu_failure("6d00")

    # INS_SIGN_UNSAFE
    app.send_apdu("8005000000")
    app.expect_apdu_failure("6d00")

    # INS_RESET
    app.send_apdu("8006000000")
    app.expect_apdu_failure("6d00")

    # INS_QUERY_AUTH_KEY
    app.send_apdu("8007000000")
    app.expect_apdu_failure("6d00")

    # INS_QUERY_MAIN_HWM
    app.send_apdu("8008000000")
    app.expect_apdu_failure("6d00")

    # INS_SETUP
    app.send_apdu("800a000000")
    app.expect_apdu_failure("6d00")

    # INS_QUERY_ALL_HWM
    app.send_apdu("800b000000")
    app.expect_apdu_failure("6d00")

    # INS_DEAUTHORIZE
    app.send_apdu("800c000000")
    app.expect_apdu_failure("6d00")

    # INS_QUERY_AUTH_KEY_WITH_CURVE
    app.send_apdu("800d000000")
    app.expect_apdu_failure("6d00")

    # INS_HMAC
    app.send_apdu("800e000000")
    app.expect_apdu_failure("6d00")

    # Unknown instruction
    app.send_apdu("80ff000000")
    app.expect_apdu_failure("6d00")

    app.assert_screen(SCREEN_HOME_DEFAULT)
    app.quit()
