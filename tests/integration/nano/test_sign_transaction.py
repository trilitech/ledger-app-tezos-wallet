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

# Operation (0): Transaction
# Fee: 0.05 XTZ
# Storage limit: 45
# Amount: 0.24 XTZ
# Destination: KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT
# Entrypoint: do
# Parameter: CAR

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        data = app.sign(DEFAULT_ACCOUNT,
                        "0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316",
                        with_hash=True,
                        path=test_name)

        app.check_signature_with_hash(
            hash="f6d5fa0e79cac216e25104938ac873ca17ee9d7f06763719293b413cf2ed475c",
            signature="f63d045a1cc9f73eee5775c5d496fa9d3aa9ae57fb97217f746a8728639795b7b2220e84ce5759ed111399ea3263d810c230d6a4fffcb6e82797c5ca673a1708",
            data=data)

        app.quit()
