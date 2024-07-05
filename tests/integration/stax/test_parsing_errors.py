#!/usr/bin/env python3

# Copyright 2023 Functori <contact@functori.com>
# Copyright 2023 TriliTech <contact@trili.tech>

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

    app.assert_screen(SCREEN_HOME_DEFAULT, True)

    # original operation : 0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316

    print("Invalid input: Unknown magic bytes")
    send_initialize_msg(app, "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f81005e0100000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316")
    app.review.tap()
    verify_err_reject_response(app, "invalid_tag")

    send_initialize_msg(app, "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f81005e03000000000000000000000000000000000000000000000000000000000000000001016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316")
    app.review.tap()
    verify_err_reject_response(app, "invalid_tag")

    print("Invalid input: 1 byte removed inside")
    send_initialize_msg(app, "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f81005d0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e010000000000000000000000000000000000000000ff02000000020316")
    app.review.tap()
    app.assert_screen("tpe_review_0_01")
    app.review.tap()
    app.assert_screen("tpe_review_0_02_partial")
    app.review.tap()
    app.assert_screen("tpe_review_0_03")
    app.review.tap()
    verify_err_reject_response(app, "invalid_tag")

    print("Invalid input: 1 byte introduce at the end")
    send_initialize_msg(app, "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f81005f0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff0200000002031645")
    app.review.tap()
    app.assert_screen("tpe_review_0_01")
    app.review.tap()
    app.assert_screen("tpe_review_0_02_full")
    app.review.tap()
    app.assert_screen("tpe_review_0_03_full")
    app.review.tap()
    app.expert_mode_splash()
    app.review.tap()
    app.assert_screen("tpe_review_0_04_full")
    app.review.tap()

    verify_err_reject_response(app, "invalid_tag")

    print("Invalid input: 1 byte introduced inside")
    send_initialize_msg(app, "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f81005f0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e0100000000000000000000000000000000000000000000ff02000000020316")
    app.review.tap()
    app.assert_screen("tpe_review_0_01")
    app.review.tap()
    app.assert_screen("tpe_review_0_02_dest_only")
    app.review.tap()
    verify_err_reject_response(app, "invalid_tag")

    # full output: 12345678901234567890123456789012345678901234567890123456789012345678901234567890
    print("Too Large input")
    send_initialize_msg(app, "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f810028050092abf8e3d9e5f8cfd9ae8a9fe5f28ea1d5b5abf1af82dae8a4b68df3d1889eb6f988f5e8d31a")
    app.review.tap()
    verify_err_reject_response(app, "too_large")

   # full output: {{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{42}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
    print("Too Deep expression")
    send_initialize_msg(app, "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f0100eb0502000000f702000000f202000000ed02000000e802000000e302000000de02000000d902000000d402000000cf02000000ca02000000c502000000c002000000bb02000000b602000000b102000000ac02000000a702000000a2020000009d02000000980200000093020000008e02000000890200000084020000007f020000007a02000000750200000070020000006b02000000660200000061020000005c02000000570200000052020000004d02000000480200000043020000003e02000000390200000034020000002f020000002a02000000250200000020020000001b020000001602000000")
    app.review.tap()
    app.assert_screen('tpe_review_too_deep_0')
    app.review.tap()
    verify_err_reject_response(app, "too_deep")

    print("wrong last packet")
    send_initialize_msg(app, "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f8100eb030000000000000000000000000000000000000000000000000000000000000000ce00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e02030400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000c63966303966323935326433343532386337333366393436313563666333396263353535363139666335353064643461363762613232303863653865383637616133643133613665663939646662653332633639373461613961323135306432316563613239633333343965353963313362393038316631")

    app.review.tap()
    app.assert_screen("tpe_review_1_01")
    # No error screen <- issue with packet framing protocol
    app.review.tap()
    app.assert_screen("tpe_review_1_02")
    app.review.tap()
    app.expect_apdu_failure("9002")

    app.assert_screen(SCREEN_HOME_DEFAULT, True)
    app.quit()
