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

    send_payload,
    verify_err_reject_response,
    reject_flow, assert_home_with_code
)

if __name__ == "__main__":
    app = tezos_app(__file__)

    app.assert_home()

    # Rejecting at 1st warning msg
    app.send_initialize_msg( "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f0100eb0502000000f702000000f202000000ed02000000e802000000e302000000de02000000d902000000d402000000cf02000000ca02000000c502000000c002000000bb02000000b602000000b102000000ac02000000a702000000a2020000009d02000000980200000093020000008e02000000890200000084020000007f020000007a02000000750200000070020000006b02000000660200000061020000005c02000000570200000052020000004d02000000480200000043020000003e02000000390200000034020000002f020000002a02000000250200000020020000001b020000001602000000")
    app.review.next()
    app.assert_screen("tbtdr_review_0")
    app.review.next()
    app.assert_screen("unsafe_operation_warning_1")
    app.review.back_to_safety.confirm()
    reject_flow(app,"9405")




    # Rejecting at 2nd warning
    app.send_initialize_msg( "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f0100eb0502000000f702000000f202000000ed02000000e802000000e302000000de02000000d902000000d402000000cf02000000ca02000000c502000000c002000000bb02000000b602000000b102000000ac02000000a702000000a2020000009d02000000980200000093020000008e02000000890200000084020000007f020000007a02000000750200000070020000006b02000000660200000061020000005c02000000570200000052020000004d02000000480200000043020000003e02000000390200000034020000002f020000002a02000000250200000020020000001b020000001602000000")
    app.review.next()
    app.assert_screen("tbtdr_review_0")
    app.review.next()
    app.assert_screen("unsafe_operation_warning_1")
    with app.fading_screen("loading_operation"):
        app.review.reject()
    app.send_apdu("800f82001211020000000c02000000070200000002002a")
    app.assert_screen("unsafe_operation_warning_2")
    with app.fading_screen("rejected"):
        app.review.back_to_safety.confirm()
    assert_home_with_code(app, "9405")


    # Rejecting at review blindsign operation
    app.send_initialize_msg( "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f0100eb0502000000f702000000f202000000ed02000000e802000000e302000000de02000000d902000000d402000000cf02000000ca02000000c502000000c002000000bb02000000b602000000b102000000ac02000000a702000000a2020000009d02000000980200000093020000008e02000000890200000084020000007f020000007a02000000750200000070020000006b02000000660200000061020000005c02000000570200000052020000004d02000000480200000043020000003e02000000390200000034020000002f020000002a02000000250200000020020000001b020000001602000000")
    app.review.next()
    app.assert_screen("tbtdr_review_0")
    app.review.next()
    app.process_blindsign_warnings( "800f82001211020000000c02000000070200000002002a")
    verify_err_reject_response(app, "tbtdr_start_review_blindsign")

    # Rejecting at blindsign review screen
    app.send_initialize_msg( "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f0100eb0502000000f702000000f202000000ed02000000e802000000e302000000de02000000d902000000d402000000cf02000000ca02000000c502000000c002000000bb02000000b602000000b102000000ac02000000a702000000a2020000009d02000000980200000093020000008e02000000890200000084020000007f020000007a02000000750200000070020000006b02000000660200000061020000005c02000000570200000052020000004d02000000480200000043020000003e02000000390200000034020000002f020000002a02000000250200000020020000001b020000001602000000")
    app.review.next()
    app.assert_screen("tbtdr_review_0")
    app.review.next()
    app.process_blindsign_warnings( "800f82001211020000000c02000000070200000002002a")
    app.assert_screen("tbtdr_start_review_blindsign")
    app.review.next()
    verify_err_reject_response(app,"tbtd_review_1")


    # Rejecting at final sign operation screen
    app.send_initialize_msg( "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f0100eb0502000000f702000000f202000000ed02000000e802000000e302000000de02000000d902000000d402000000cf02000000ca02000000c502000000c002000000bb02000000b602000000b102000000ac02000000a702000000a2020000009d02000000980200000093020000008e02000000890200000084020000007f020000007a02000000750200000070020000006b02000000660200000061020000005c02000000570200000052020000004d02000000480200000043020000003e02000000390200000034020000002f020000002a02000000250200000020020000001b020000001602000000")
    app.review.next()
    app.assert_screen("tbtdr_review_0")
    app.review.next()
    app.process_blindsign_warnings( "800f82001211020000000c02000000070200000002002a")
    app.assert_screen("tbtdr_start_review_blindsign")
    app.review.next()
    app.assert_screen("tbtd_review_1")
    app.review.next()
    verify_err_reject_response(app,"operation_sign_blindsign")

    app.quit()
