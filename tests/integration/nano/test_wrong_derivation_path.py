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

from utils.account import Account, SIGNATURE_TYPE
from utils.app import Screen
from utils.backend import INS, StatusCode

def test_wrong_derivation_path(app):
    wrong_number_index_account = Account(
        bytes.fromhex("058000002c800006c18000000080000000"),
        SIGNATURE_TYPE.ED25519,
        "__unused__")
    wrong_length_account = Account(
        bytes.fromhex("048000002c800006c180000000800000"),
        SIGNATURE_TYPE.ED25519,
        "__unused__")
    too_much_index_account = Account(
        bytes.fromhex("0b8000002c800006c1800000008000000080000000800000008000000080000000800000008000000080000000"),
        SIGNATURE_TYPE.ED25519,
        "__unused__")

    for account in [wrong_number_index_account,
                    wrong_length_account,
                    too_much_index_account]:
        for sender in [lambda account: app.backend.get_public_key(account, with_prompt=False),
                       lambda account: app.backend.get_public_key(account, with_prompt=True),
                       lambda account: app.backend._ask_sign(INS.SIGN, account),
                       lambda account: app.backend._ask_sign(INS.SIGN_WITH_HASH, account)]:

            app.assert_screen(Screen.Home)

            with app.expect_apdu_failure(StatusCode.WRONG_LENGTH_FOR_INS):
                sender(account)

    app.quit()
