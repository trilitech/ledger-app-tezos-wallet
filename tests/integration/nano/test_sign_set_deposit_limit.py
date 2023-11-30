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

# Operation (0): Set deposit limit
# Fee: 0.06 XTZ
# Storage limit: 4
# Staking limit: 0.02 XTZ

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        data = app.sign(DEFAULT_ACCOUNT,
                        "03000000000000000000000000000000000000000000000000000000000000000070027c252d3806e6519ed064026bdb98edf866117331e0d40304f80204ffa09c01",
                        with_hash=True,
                        path=test_name)

        app.check_signature_with_hash(
            hash="8b4456454de1b3c41f5ea45e711893df26fabe9427048b95fda4276d5cf76ff6",
            signature="069c2cd9fe167a52cc21611a0f59465784f4fce94211aab9fee6309c8e8bf5cbcf1a3e3102d0825b5acaf341656b1c2078850f7d3a6749cc47f74688fbe2c30e",
            data=data)

        app.quit()
