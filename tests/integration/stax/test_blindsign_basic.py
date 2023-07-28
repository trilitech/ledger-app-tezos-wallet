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

if __name__ == "__main__":
    app = stax_app()

    # Switch to blindsign mode
    app.assert_screen(SCREEN_HOME_DEFAULT)

    app.welcome.settings()
    app.assert_screen(SCREEN_INFO_PAGE)

    app.info.next()
    app.assert_screen("settings_blindsigning_off")

    app.settings_toggle_blindsigning()
    app.assert_screen("settings_blindsigning_on")

    app.info.multi_page_exit()
    app.assert_screen("home_bs_enabled_clearsign")

    app.welcome.action()
    app.assert_screen("home_bs_enabled_blindsign")

    # Blindsign a hash
    app.start_loading_operation("800f000011048000002c800006c18000000080000000")

    app.send_apdu("800f0100eb0300000000000000000000000000000000000000000000000000000000000000006c00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e0203040001000000000000000000000000000000000000000000ff01000001070200000102076501000000015b076501000000015a0765010000000159076501000000015807650100000001570765010000000156076501000000015507650100000001540765010000000153076501000000015207650100000001510765010000000150076501000000014f076501000000014e076501000000014d076501000000014c076501000000014b0765010000")
    app.expect_apdu_return("9000")

    app.send_apdu("800f82007500014a0765010000000149076501000000014807650100000001470765010000000146076501000000014507650100000001440765010000000143076501000000014202000000000765000a0765000907650008076500070765000607650005076500040765000307650002076500010200000000")

    app.assert_screen("request_blindsign_manager")
    app.review.tap()

    app.assert_screen("review_blindsign_manager_basic")
    app.review.tap()

    app.assert_screen("review_blindsign_approve")

    expected_apdu = "a3a979004f354cb391acac7515ffde7e5613962007534a7fc2cb534300fe4bc96c54b80041ef3d764b79e17a11524b68085ad6f945bf21e5d17e7657763b7d355f3a816e11d10b9cf6ba59e1535a02a541561852b19767b3922d6e00c8616d0e9000"
    app.review_confirm_signing(expected_apdu)

    app.assert_screen("home_bs_enabled_blindsign")
    app.welcome.quit()
