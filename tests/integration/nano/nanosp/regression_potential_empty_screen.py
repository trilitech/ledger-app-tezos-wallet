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

import os
import sys

file_path=os.path.abspath(__file__)
dir_path=os.path.dirname(file_path)
root_path=os.path.dirname(dir_path)
sys.path.append(root_path)

from utils.apdu import *
from utils.app import *

# Operation (0): Transfer ticket
# Fee: 0.01 XTZ
# Storage limit: 4
# Contents: UNPAIR
# Type: pair "1" 2
# Ticketer: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Amount: 1
# Destination: KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT
# Entrypoint: S
#
# S
# S
# S

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        data = app.sign(DEFAULT_ACCOUNT,
                        "0300000000000000000000000000000000000000000000000000000000000000009e00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e02030400000002037a0000000a076501000000013100020000ffdd6102321bc251e4a5190ad5b12b251069d9b4010100000000000000000000000000000000000000000000000008530a0a530a530a53",
                        with_hash=True,
                        path=test_name)

        app.check_signature_with_hash(
            hash="ba220e5b9af0fa350d127665049ef6dcc85304a6bd62fcc5e4f12752092af1f7",
            signature="03399e1639e7884f86b83714e5eea2acdc56d3449f029e7258ef3bbbd35f449105d9545c3c62f7ffa088d3dfebcfa38cd316e2b4d4067cf288e9e275b8fe6901",
            data=data)

        app.quit()
