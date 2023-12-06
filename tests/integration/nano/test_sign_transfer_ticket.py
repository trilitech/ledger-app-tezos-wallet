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

from pathlib import Path

from utils.app import nano_app, Screen, DEFAULT_ACCOUNT
from utils.message import Message

# Operation (0): Transfer ticket
# Fee: 0.01 XTZ
# Storage limit: 4
# Contents: UNPAIR
# Type: pair "1" 2
# Ticketer: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Amount: 1
# Destination: KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT
# Entrypoint: default

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        message = Message.from_bytes("0300000000000000000000000000000000000000000000000000000000000000009e00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e02030400000002037a0000000a076501000000013100020000ffdd6102321bc251e4a5190ad5b12b251069d9b401010000000000000000000000000000000000000000000000000764656661756c74")

        data = app.sign(DEFAULT_ACCOUNT,
                        message,
                        with_hash=True,
                        path=test_name)

        app.checker.check_signature(
            account=DEFAULT_ACCOUNT,
            message=message,
            with_hash=True,
            data=data)

        app.quit()
