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
    app = stax_app(__file__)

    # Switch to blindsign mode
    app.assert_home()

    # Blindsigning disabled (default state)
    send_initialize_msg(app, "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f810028050092abf8e3d9e5f8cfd9ae8a9fe5f28ea1d5b5abf1af82dae8a4b68df3d1889eb6f988f5e8d31a")
    app.review.next()

    app.assert_screen("too_large_enable_blindsign")
    with app.fading_screen("blindsign_enabled"):
        app.review.enable_blindsign.confirm()

    app.assert_screen("blindsign_warning")
    app.review.next()

    app.assert_screen("tbtl_review_1")
    app.review.next()
    app.assert_screen("operation_sign")

    expected_apdu = "ef565fa445d815cd77518a4d14ce90b7a536627455f0930c9dbfa22a75d478d83e2bcb333ba0d639dd28c1b77c5860e552ab02092a50a57f1424f573278230ab8ba81d8a40956415278a27e3f28cae64d1f1f13bf613e6e9a57035e9e14511029000"
    app.review_confirm_signing(expected_apdu)

    app.assert_home()

    # Blindsign enabled
    app.welcome.settings()
    app.assert_settings(blindsigning=True)
    # blind sign will be on because of previous test run.
    app.settings.exit()

    app.assert_home()

    send_initialize_msg(app, "800f000011048000002c800006c18000000080000000")
    send_payload(app,"800f810028050092abf8e3d9e5f8cfd9ae8a9fe5f28ea1d5b5abf1af82dae8a4b68df3d1889eb6f988f5e8d31a")
    app.review.next()
    app.assert_screen("too_large_blindsign_notify")
    app.review.next()

    app.assert_screen("blindsign_warning")
    app.review.next()

    app.assert_screen("tbtl_review_1")
    app.review.next()
    app.assert_screen("operation_sign")

    expected_apdu = "ef565fa445d815cd77518a4d14ce90b7a536627455f0930c9dbfa22a75d478d83e2bcb333ba0d639dd28c1b77c5860e552ab02092a50a57f1424f573278230ab8ba81d8a40956415278a27e3f28cae64d1f1f13bf613e6e9a57035e9e14511029000"
    app.review_confirm_signing(expected_apdu)

    app.quit()
