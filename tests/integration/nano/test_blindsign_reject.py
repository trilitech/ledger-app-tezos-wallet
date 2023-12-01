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

from utils.app import nano_app, Screen_text, send_and_navigate, DEFAULT_ACCOUNT
from utils.backend import StatusCode
from utils.message import Message

# Expression: 12345678901234567890123456789012345678901234567890123456789012345678901234567890
# is too large

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        expression = Message.from_bytes("050092abf8e3d9e5f8cfd9ae8a9fe5f28ea1d5b5abf1af82dae8a4b68df3d1889eb6f988f5e8d31a")

        app.setup_blind_signing()

        app._failing_signing(DEFAULT_ACCOUNT,
                             expression,
                             with_hash=False,
                             text=Screen_text.Sign_reject,
                             status_code=StatusCode.PARSE_ERROR,
                             path=Path(test_name) / "reject_from_clear")

        def expected_failure_send() -> bytes:
            with app.expect_apdu_failure(StatusCode.REJECT):
                app.backend.sign(DEFAULT_ACCOUNT, expression, with_hash=False)
            return b''

        def navigate() -> None:
            app.navigate_until_text(Screen_text.Blind_switch, Path(test_name) / "reject_from_blind" / "clear")
            app.navigate_until_text(Screen_text.Sign_reject, Path(test_name) / "reject_from_blind" / "blind")

        send_and_navigate(
            send=expected_failure_send,
            navigate=navigate)

        app.quit()
