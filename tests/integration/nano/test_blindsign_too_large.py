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

# Expression: 12345678901234567890123456789012345678901234567890123456789012345678901234567890

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        data = app.blind_sign(DEFAULT_ACCOUNT,
                              "050092abf8e3d9e5f8cfd9ae8a9fe5f28ea1d5b5abf1af82dae8a4b68df3d1889eb6f988f5e8d31a",
                              with_hash=True,
                              path=test_name)

        app.check_signature_with_hash(
            hash="ef565fa445d815cd77518a4d14ce90b7a536627455f0930c9dbfa22a75d478d8",
            signature="3e2bcb333ba0d639dd28c1b77c5860e552ab02092a50a57f1424f573278230ab8ba81d8a40956415278a27e3f28cae64d1f1f13bf613e6e9a57035e9e1451102",
            data=data)

        app.quit_blind()
