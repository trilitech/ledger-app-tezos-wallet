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

from utils.app import Screen, Screen_text, DEFAULT_ACCOUNT
from utils.backend import StatusCode
from utils.message import Message

# Operation (0): Transaction
# Fee: 0.05 XTZ
# Storage limit: 45
# Amount: 0.24 XTZ
# Destination: KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT
# Entrypoint: do
# Parameter: CAR

def test_too_complex_transaction(app):
    test_name = Path(__file__).stem

    app.assert_screen(Screen.Home)

    message = Message.from_bytes("0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316")

    app._failing_signing(DEFAULT_ACCOUNT,
                         message,
                         with_hash=True,
                         text=Screen_text.Back_home,
                         status_code=StatusCode.REJECT,
                         path=test_name)

    app.quit()
