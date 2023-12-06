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

from utils.account import Account
from utils.app import nano_app, send_and_navigate, Screen, Screen_text, DEFAULT_ACCOUNT
from utils.message import Message

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        def check_sign_with_small_packet(
                account: Account,
                message: Message,
                path: str) -> None:

            app.assert_screen(Screen.Home)

            data = send_and_navigate(
                send=(lambda: app.backend.sign(account, message, apdu_size=10)),
                navigate=(lambda: app.navigate_until_text(Screen_text.Sign_accept, path)))

            app.checker.check_signature(
                account,
                message,
                with_hash=False,
                data=data)

            app.assert_screen(Screen.Home)

        check_sign_with_small_packet(
            account=DEFAULT_ACCOUNT,
            message=Message.from_bytes("0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316"),
            path=test_name)

        app.quit()
