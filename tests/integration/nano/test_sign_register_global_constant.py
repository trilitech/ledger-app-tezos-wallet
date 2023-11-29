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

# Operation (0): Register global constant
# Fee: 0.01 XTZ
# Storage limit: 4
# Value: Pair "1" 2

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        data = app.sign(DEFAULT_ACCOUNT,
                        "0300000000000000000000000000000000000000000000000000000000000000006f00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e0203040000000a07070100000001310002",
                        with_hash=True,
                        path=test_name)

        app.check_signature_with_hash(
            hash="bb38ac8ad80f5280b3f4e006d5656a8c8f6192994c86dcd160e4f5977332ccb7",
            signature="528f3b3ff8b2ed2095f23add2c7409b05cbfdf1dae96ad7436083429bad30cd867912a42fe9f5eddc153cb847f13ffa4e7979ce78b833f8c7cbf3b8345591802",
            data=data)

        app.quit()
