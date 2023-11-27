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

def assert_home_with_code(app, code):
    app.assert_screen(SCREEN_HOME_DEFAULT)
    app.expect_apdu_failure(code)

def send_first(app, apdu):
    app.send_apdu(apdu)
    app.expect_apdu_return("9000")

    app.assert_screen("review_request_sign_operation");

def send_second(app, apdu):
    app.send_apdu(apdu)
    app.assert_screen("review_request_sign_operation");

if __name__ == "__main__":
    app = stax_app(__file__)

    app.assert_screen(SCREEN_HOME_DEFAULT)

    # original operation : 0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316

    # Unknown magic bytes
    send_first(app, "800f000011048000002c800006c18000000080000000")
    send_second(app, "800f81005e0100000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316")

    with app.review_parsing_error("invalid_tag"):
        app.review.tap()
    assert_home_with_code(app, "9405")

    # Unknown operation
    send_first(app, "800f000011048000002c800006c18000000080000000")
    send_second(app, "800f81005e03000000000000000000000000000000000000000000000000000000000000000001016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316")

    with app.review_parsing_error("invalid_tag"):
        app.review.tap()
    assert_home_with_code(app, "9405")

    # 1 byte remove inside
    send_first(app, "800f000011048000002c800006c18000000080000000")
    send_second(app, "800f81005d0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e010000000000000000000000000000000000000000ff02000000020316")

    app.review.tap()
    app.assert_screen("tpe_review_0_01")

    app.review.tap()
    app.assert_screen("tpe_review_0_02_partial")

    with app.review_parsing_error("invalid_tag"):
        app.review.tap()
    assert_home_with_code(app, "9405")

    # 1 byte introduce at the end
    send_first(app, "800f000011048000002c800006c18000000080000000")
    send_second(app, "800f81005f0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff0200000002031645")

    app.review.tap()
    app.assert_screen("tpe_review_0_01")

    app.review.tap()
    app.assert_screen("tpe_review_0_02_full")

    with app.review_parsing_error("invalid_tag"):
        app.review.tap()
    assert_home_with_code(app, "9405")

    # 1 byte introduce inside
    send_first(app, "800f000011048000002c800006c18000000080000000")
    send_second(app, "800f81005f0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e0100000000000000000000000000000000000000000000ff02000000020316")

    app.review.tap()
    app.assert_screen("tpe_review_0_01")

    app.review.tap()
    app.assert_screen("tpe_review_0_02_dest_only")

    with app.review_parsing_error("invalid_tag"):
        app.review.tap()
    assert_home_with_code(app, "9405")

    # full output: 12345678901234567890123456789012345678901234567890123456789012345678901234567890
    send_first(app, "800f000011048000002c800006c18000000080000000")
    send_second(app, "800f810028050092abf8e3d9e5f8cfd9ae8a9fe5f28ea1d5b5abf1af82dae8a4b68df3d1889eb6f988f5e8d31a")

    with app.review_parsing_error("too_large"):
        app.review.tap()
    assert_home_with_code(app, "9405")

    # full output: {{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{42}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
    send_first(app, "800f000011048000002c800006c18000000080000000")
    send_second(app, "800f0100eb0502000000f702000000f202000000ed02000000e802000000e302000000de02000000d902000000d402000000cf02000000ca02000000c502000000c002000000bb02000000b602000000b102000000ac02000000a702000000a2020000009d02000000980200000093020000008e02000000890200000084020000007f020000007a02000000750200000070020000006b02000000660200000061020000005c02000000570200000052020000004d02000000480200000043020000003e02000000390200000034020000002f020000002a02000000250200000020020000001b020000001602000000")

    with app.review_parsing_error("too_deep"):
        app.review.tap()
        app.assert_screen("tpe_review_too_deep_0")
        app.review.tap()
    assert_home_with_code(app, "9405")

    # wrong last packet
    send_first(app, "800f000011048000002c800006c18000000080000000")
    send_second(app, "800f8100eb030000000000000000000000000000000000000000000000000000000000000000ce00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e02030400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000c63966303966323935326433343532386337333366393436313563666333396263353535363139666335353064643461363762613232303863653865383637616133643133613665663939646662653332633639373461613961323135306432316563613239633333343965353963313362393038316631")

    app.review.tap()
    app.assert_screen("tpe_review_1_01")

    # No error screen <- issue with packet framing protocol
    app.review.tap()
    app.expect_apdu_failure("9002")

    app.assert_screen(SCREEN_HOME_DEFAULT)
    app.quit()
