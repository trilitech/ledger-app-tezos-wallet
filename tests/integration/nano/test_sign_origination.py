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

# Operation (0): Origination
# Fee: 0.01 XTZ
# Storage limit: 4
# Balance: 0.5 XTZ
# Delegate: Field unset
# Code: UNPAIR
# Storage: pair "1" 2

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        data = app.sign(DEFAULT_ACCOUNT,
                        "0300000000000000000000000000000000000000000000000000000000000000006d00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e020304a0c21e0000000002037a0000000a07650100000001310002",
                        with_hash=True,
                        path=test_name)

        app.check_signature_with_hash(
            hash="4c6b124e010e5701ea67043d176136234a1dc9c3514dbc512209bd0a28033dcf",
            signature="621fcc5d36f84f3fffd9177278499f16bb363ae4573227105fd584f58d1be77ebcfc17250cf575b8aad31951f18e35a3252f10583768abf4f2511bd84669c003",
            data=data)

        app.quit()
