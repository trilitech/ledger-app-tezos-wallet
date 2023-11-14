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

from utils.apdu import *
from utils.app import *

# Operation (0): Reveal
# Fee: 0.01 tz
# Storage limit: 4
# Public key: edpkuXX2VdkdXzkN11oLCb8Aurdo1BTAtQiK8ZY9UPj2YMt3AHEpcY

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        data = app.sign_with_hash(DEFAULT_ACCOUNT,
                                  "0300000000000000000000000000000000000000000000000000000000000000006b00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e02030400747884d9abdf16b3ab745158925f567e222f71225501826fa83347f6cbe9c393",
                                  path=test_name)

        app.check_signature_with_hash(
            hash="28a91fe25dca9feed9a746d2825f113b0b7c0c534853d4d9e8d37f3a29119a3e",
            signature="dc717466cf0fb90cd9ea8fcaf5252a52ae865901d28f54128498a94588530a48eb56c343497ff3f69a671f97b4a5e4dec0f7afb443f6658f62610287c829b608",
            data=data)

        app.quit()
