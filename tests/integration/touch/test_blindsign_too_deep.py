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

from utils import (
    tezos_app,
    BlindsigningType,
    BlindsigningStatus,
)

if __name__ == "__main__":
    app = tezos_app(__file__)

    app.assert_home()

    app.send_initialize_msg( "800f000011048000002c800006c18000000080000000")
    app.send_apdu("800f0100eb0502000000f702000000f202000000ed02000000e802000000e302000000de02000000d902000000d402000000cf02000000ca02000000c502000000c002000000bb02000000b602000000b102000000ac02000000a702000000a2020000009d02000000980200000093020000008e02000000890200000084020000007f020000007a02000000750200000070020000006b02000000660200000061020000005c02000000570200000052020000004d02000000480200000043020000003e02000000390200000034020000002f020000002a02000000250200000020020000001b020000001602000000")
    app.review.next()

    app.assert_screen("tbtd_review_0")
    app.review.next()

    app.process_blindsign_warnings("800f82001211020000000c02000000070200000002002a")

    app.assert_screen("tbtd_start_review_blindsign")
    app.review.next()
    app.assert_screen("tbtd_review_1")
    expected_apdu = "93070b00990e4cf29c31f6497307bea0ad86a9d0dc08dba8b607e8dc0e23652f8309e41ed87ac1d33006806b688cfcff7632c4fbe499ff3ea4983ae4f06dea7790ec25db045689bca2c63967b5c563aabff86c4ef163bff92af3bb2ca9392d099000"
    app.review_confirm_signing(expected_apdu, blindsigning_type=BlindsigningType.BLINDSIGN)

    app.assert_home()

    app.set_blindsigning_status(BlindsigningStatus.ON)
    app.send_initialize_msg( "800f000011048000002c800006c18000000080000000")
    app.send_apdu("800f0100eb0502000000f702000000f202000000ed02000000e802000000e302000000de02000000d902000000d402000000cf02000000ca02000000c502000000c002000000bb02000000b602000000b102000000ac02000000a702000000a2020000009d02000000980200000093020000008e02000000890200000084020000007f020000007a02000000750200000070020000006b02000000660200000061020000005c02000000570200000052020000004d02000000480200000043020000003e02000000390200000034020000002f020000002a02000000250200000020020000001b020000001602000000")
    app.review.next()

    app.assert_screen("tbtd_review_blindsign_on_0")
    app.review.skip() # skip button
    app.review.enable_skip.confirm()
    app.assert_screen("too_deep_blindsign_warning")
    with app.fading_screen("loading_operation"):
        app.review.enable_blindsign.reject()
    app.expect_apdu_return("9000")
    app.send_apdu("800f82001211020000000c02000000070200000002002a")
    app.assert_screen("blindsign_warning_ledger_1")
    app.review.enable_blindsign.reject()
    app.assert_screen("blindsign_review_transaction")
    app.review.next()
    app.assert_screen("tbtd_review_blindsign_0")
    app.review.next()
    app.review_confirm_signing(expected_apdu, blindsigning_type=BlindsigningType.BLINDSIGN)

    app.assert_home()


    app.quit()
