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

from utils.app import *
from utils.backend import *

# Operation (0): Ballot
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Period: 32
# Proposal: ProtoALphaALphaALphaALphaALphaALphaALpha61322gcLUGH
# Ballot: yay

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        message = "0300000000000000000000000000000000000000000000000000000000000000000600ffdd6102321bc251e4a5190ad5b12b251069d9b4000000200bcd7b2cadcd87ecb0d5c50330fb59feed7432bffecede8a09a2b86cfb33847b00"

        data = app.sign(DEFAULT_ACCOUNT,
                        message,
                        with_hash=True,
                        path=test_name)

        app.check_signature_with_hash(
            account=DEFAULT_ACCOUNT,
            message=message,
            hash="80a74079a1911a95b4aea45d6b29321e1705165194521bf74982c4b8c5768240",
            data=data)

        app.quit()
