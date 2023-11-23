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

# Operation (0): Increase paid storage
# Fee: 0.01 XTZ
# Storage limit: 4
# Amount: 5
# Destination: KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        data = app.sign_with_hash(DEFAULT_ACCOUNT,
                                  "0300000000000000000000000000000000000000000000000000000000000000007100ffdd6102321bc251e4a5190ad5b12b251069d9b4904e0203040501000000000000000000000000000000000000000000",
                                  path=test_name)

        app.check_signature_with_hash(
            hash="48ab3de08b4a53ffe8cb8984cb5e0174082496321d5f9644ec8a3f01f3b2176f",
            signature="6782addb600b8b796195b2591e4f240091d478214661ff6728b392f0a84d8eca66ddcb5d5c83838642c0335f7d6b797c835ea8f465550e8c1d5ec64d87193b07",
            data=data)

        app.quit()
