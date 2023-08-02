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

    app.assert_screen(HOME_TEXT)

    app.send_apdu("8003000011048000002c800006c18000000080000000")
    app.assert_screen([TEZOS_LOGO, "Verify Tezos", "address"])

    app.provide_pk.tap()
    app.assert_screen(["Address", "tz1dyX3B1CFYa2DfdFL", "yPtiJCfQRUgPVME6E", "Show as QR"])

    app.provide_pk.confirm()
    app.assert_screen(["ADDRESS", "VERIFIED"])

    app.provide_pk.tap()
    app.expect_apdu_return("2102747884d9abdf16b3ab745158925f567e222f71225501826fa83347f6cbe9c3939000")

    app.assert_screen(HOME_TEXT)
    app.welcome.quit()
