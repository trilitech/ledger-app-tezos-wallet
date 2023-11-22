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

# Operation (0): Transaction
# Fee: 0.01 XTZ
# Storage limit: 4
# Amount: 0 XTZ
# Destination: KT1GW4QHn66m7WWWMWCMNaWmGYpCRbg5ahwU
# Entrypoint: root
# Parameter: 0

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        data = app.sign_with_hash(DEFAULT_ACCOUNT,
                                  "0300000000000000000000000000000000000000000000000000000000000000006c00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e020304000156dcfb211fa76c525fd7c4566c09a5e3e4d5b81000ff01000000020000",
                                  test_name)

        app.check_signature_with_hash(
            hash="24aeac1f45f96ff13503b1354f8def563224633196aafb62e55df30c3894153f",
            signature="33496d120072845be599fd1811376ab93ec9f14130cb434d17ccc2563eb68b561b6c5e8b4893b588dea011cd0e2f49b992b750b78262cf318dd35e1945e87e00",
            data=data)

        app.quit()
