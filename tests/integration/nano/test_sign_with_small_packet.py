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

"""Check signing using small packet instead of full size packets"""

from pathlib import Path

from utils.account import Account
from utils.app import send_and_navigate, Screen, ScreenText, TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import Message, Transaction

def test_sign_with_small_packet(app: TezosAppScreen):
    """Check signing using small packet instead of full size packets"""
    test_name = Path(__file__).stem

    app.setup_expert_mode()

    def check_sign_with_small_packet(
            account: Account,
            message: Message,
            path: str) -> None:

        app.assert_screen(Screen.HOME)

        data = send_and_navigate(
            send=lambda: app.backend.sign(account, message, apdu_size=10),
            navigate=lambda: app.navigate_until_text(ScreenText.SIGN_ACCEPT, path))

        app.checker.check_signature(
            account,
            message,
            with_hash=False,
            data=data)

    message = Transaction(
        source = 'tz2JPgTWZZpxZZLqHMfS69UAy1UHm4Aw5iHu',
        fee = 50000,
        counter = 8,
        gas_limit = 54,
        storage_limit = 45,
        destination = 'KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT',
        amount = 240000,
        entrypoint = 'do',
        parameter = {'prim': 'CAR'}
    )

    check_sign_with_small_packet(
        account=DEFAULT_ACCOUNT,
        message=message,
        path=test_name)

    app.quit()
