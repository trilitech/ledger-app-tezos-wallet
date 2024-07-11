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

def toggle_blindsigning(app):
    app.assert_home()
    app.welcome.settings()
    app.settings.toggle_blindsigning()
    app.settings.exit()
    app.assert_home()

if __name__ == "__main__":
    app = stax_app(__file__)

    app.assert_home()

    # Rejecting at error screen (blindsign disabled)
    send_initialize_msg(app, "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f0100eb0502000000f702000000f202000000ed02000000e802000000e302000000de02000000d902000000d402000000cf02000000ca02000000c502000000c002000000bb02000000b602000000b102000000ac02000000a702000000a2020000009d02000000980200000093020000008e02000000890200000084020000007f020000007a02000000750200000070020000006b02000000660200000061020000005c02000000570200000052020000004d02000000480200000043020000003e02000000390200000034020000002f020000002a02000000250200000020020000001b020000001602000000")
    app.review.next()
    app.assert_screen("too_deep_expression")
    app.review.next()
    verify_err_reject_response(app, "too_deep_enable_blindsign")


    # Rejecting at error screen (blindsign enabled)
    toggle_blindsigning(app) # Enable blindsigning
    send_initialize_msg(app, "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f0100eb0502000000f702000000f202000000ed02000000e802000000e302000000de02000000d902000000d402000000cf02000000ca02000000c502000000c002000000bb02000000b602000000b102000000ac02000000a702000000a2020000009d02000000980200000093020000008e02000000890200000084020000007f020000007a02000000750200000070020000006b02000000660200000061020000005c02000000570200000052020000004d02000000480200000043020000003e02000000390200000034020000002f020000002a02000000250200000020020000001b020000001602000000")
    app.review.next()
    app.assert_screen("too_deep_expression")
    app.review.next()
    verify_err_reject_response(app, "too_deep_blindsign_notify")

    # Rejecting at blindsigning splash screen
    toggle_blindsigning(app) # Disable blindsigning at home
    send_initialize_msg(app, "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f0100eb0502000000f702000000f202000000ed02000000e802000000e302000000de02000000d902000000d402000000cf02000000ca02000000c502000000c002000000bb02000000b602000000b102000000ac02000000a702000000a2020000009d02000000980200000093020000008e02000000890200000084020000007f020000007a02000000750200000070020000006b02000000660200000061020000005c02000000570200000052020000004d02000000480200000043020000003e02000000390200000034020000002f020000002a02000000250200000020020000001b020000001602000000")
    app.review.next()
    app.assert_screen("too_deep_expression")
    app.review.next()
    app.assert_screen("too_deep_enable_blindsign")
    app.review.enable_blindsign.confirm()
    app.assert_screen("blindsign_enabled")
    app.review.tap()
    verify_err_reject_response(app,"blindsign_warning")

    # Rejecting at blindsign review screen
    toggle_blindsigning(app)   # Disable blindsigning at home
    send_initialize_msg(app, "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f0100eb0502000000f702000000f202000000ed02000000e802000000e302000000de02000000d902000000d402000000cf02000000ca02000000c502000000c002000000bb02000000b602000000b102000000ac02000000a702000000a2020000009d02000000980200000093020000008e02000000890200000084020000007f020000007a02000000750200000070020000006b02000000660200000061020000005c02000000570200000052020000004d02000000480200000043020000003e02000000390200000034020000002f020000002a02000000250200000020020000001b020000001602000000")
    app.review.next()
    app.assert_screen("too_deep_expression")
    app.review.next()
    app.assert_screen("too_deep_enable_blindsign")
    app.review.enable_blindsign.confirm()
    app.assert_screen("blindsign_enabled")
    app.review.tap()
    app.assert_screen("blindsign_warning")
    app.welcome.client.pause_ticker()
    app.review.next()
    app.assert_screen("loading_operation")
    app.welcome.client.resume_ticker()
    app.send_apdu("800f82001211020000000c02000000070200000002002a")
    verify_err_reject_response(app,"tbtd_review_1")


    # Rejecting at final sign operation screen
    toggle_blindsigning(app)   # Disable blindsigning at home
    send_initialize_msg(app, "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f0100eb0502000000f702000000f202000000ed02000000e802000000e302000000de02000000d902000000d402000000cf02000000ca02000000c502000000c002000000bb02000000b602000000b102000000ac02000000a702000000a2020000009d02000000980200000093020000008e02000000890200000084020000007f020000007a02000000750200000070020000006b02000000660200000061020000005c02000000570200000052020000004d02000000480200000043020000003e02000000390200000034020000002f020000002a02000000250200000020020000001b020000001602000000")
    app.review.next()
    app.assert_screen("too_deep_expression")
    app.review.next()
    app.assert_screen("too_deep_enable_blindsign")
    app.review.enable_blindsign.confirm()
    app.assert_screen("blindsign_enabled")
    app.review.tap()
    app.assert_screen("blindsign_warning")
    app.welcome.client.pause_ticker()
    app.review.next()
    app.assert_screen("loading_operation")
    app.welcome.client.resume_ticker()
    app.send_apdu("800f82001211020000000c02000000070200000002002a")
    app.assert_screen("tbtd_review_1")
    app.review.next()
    verify_err_reject_response(app,"operation_sign")

    app.quit()
