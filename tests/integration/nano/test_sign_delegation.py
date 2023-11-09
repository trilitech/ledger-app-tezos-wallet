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

# Operation (0): Delegation
# Fee: 0.2 tz
# Storage limit: 889
# Delegate: tz1TmFPVZsGQ8MnrBJtnECJgkFUwLa6EWYDm

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        data = app.sign_with_hash(DEFAULT_ACCOUNT,
                                  "0300000000000000000000000000000000000000000000000000000000000000006e01774d99da021b92d8c3dfc2e814c7658440319be2c09a0cf40509f906ff00591e842444265757d6a65e3670ca18b5e662f9c0",
                                  path=test_name)

        app.check_signature_with_hash(
            hash="a6b0ce461c2c49d22cc9ec552e2cec1f5648c724012d53384f07a5134366dcc0",
            signature="be83b0502234bb618de1b8e5585c3344332284858cc799f3e2f58820ba2e6f66c2403d3084b0ab6ae6400733a912448e7614fbab139b532f081da7bcf12f5b02",
            data=data)

        app.quit()
