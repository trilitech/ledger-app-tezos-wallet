#!/usr/bin/env python3
# Copyright 2023 Trilitech <contact@trili.tech>

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

# full input: 0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316
# full output: CAR
# signer: tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E
# path: m/44'/1729'/0'/0'


def sign_transfer_initialize(app):
    app.assert_screen(SCREEN_HOME_DEFAULT)
    app.send_apdu("800f000011048000002c800006c18000000080000000");
    app.expect_apdu_return("9000");
    app.assert_screen("review_request_sign_operation");
    app.send_apdu("800f81005e0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316");
    app.review.tap()
    app.assert_screen(f"tst_review_001")
    app.review.tap()
    app.assert_screen("tst_review_002")
    app.review.tap()
    app.assert_screen("tst_review_003")



if __name__ == "__main__":
    app = stax_app(__file__)
    #  Reject from enable expert mode
    sign_transfer_initialize(app)
    app.review.tap()
    verify_reject_response(app,"enable_expert_mode")

    # Reject from enabled expert mode
    sign_transfer_initialize(app)
    app.review.tap()
    app.assert_screen("enable_expert_mode")
    app.welcome.client.finger_touch(BUTTON_ABOVE_LOWER_MIDDLE.x, BUTTON_ABOVE_LOWER_MIDDLE.y)
    verify_reject_response(app, "enabled_expert_mode")

    #- Reject from expert mode splash screen

    # In previous test, expert mode was enabled, disable it here.
    app.welcome.info()
    app.assert_screen(SCREEN_INFO_PAGE)
    app.info.next()
    app.assert_screen("settings_expert_on_blindsigning_off")
    app.settings_toggle_expert_mode()
    app.assert_screen("settings_expert_blindsiging_off")
    app.info.multi_page_exit()

    sign_transfer_initialize(app)
    app.review.tap()
    app.enable_expert_mode()
    app.review.tap()
    verify_reject_response(app, "expert_mode_splash")

    # Now with expert mode enabled, reject from splash screen.

    sign_transfer_initialize(app)
    app.review.tap()
    verify_reject_response(app, "expert_mode_splash")
