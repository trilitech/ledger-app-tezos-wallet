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

from utils.app import Screen, DEFAULT_ACCOUNT
from utils.message import Message

# Operation (0): Ballot
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Period: 32
# Proposal: ProtoALphaALphaALphaALphaALphaALphaALpha61322gcLUGH
# Ballot: yay

def test_sign_ballot(app):
    test_name = Path(__file__).stem

    app.assert_screen(Screen.Home)

    message = Message.from_bytes("0300000000000000000000000000000000000000000000000000000000000000000600ffdd6102321bc251e4a5190ad5b12b251069d9b4000000200bcd7b2cadcd87ecb0d5c50330fb59feed7432bffecede8a09a2b86cfb33847b00")

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
