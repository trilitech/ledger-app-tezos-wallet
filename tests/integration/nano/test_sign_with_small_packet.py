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

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        def sign_with_small_packet(account: Account, message: str, path: str) -> bytes:

            return send_and_navigate(
                send=(lambda: app.backend._sign(INS.SIGN, DEFAULT_ACCOUNT, message, apdu_size=10)),
                navigate=(lambda: app.navigate_until_text("Accept", path)))

        app.assert_screen(Screen.Home)

        data = sign_with_small_packet(
            account=DEFAULT_ACCOUNT,
            message="0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316",
            path=test_name)

        app.check_signature(
            "f63d045a1cc9f73eee5775c5d496fa9d3aa9ae57fb97217f746a8728639795b7b2220e84ce5759ed111399ea3263d810c230d6a4fffcb6e82797c5ca673a1708",
            data)

        app.quit()
