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

def test_get_pk(app):

    accounts = [
        Account("m/44'/1729'/0'/0'",
                SIGNATURE_TYPE.ED25519,
                "edpkuXX2VdkdXzkN11oLCb8Aurdo1BTAtQiK8ZY9UPj2YMt3AHEpcY"),
        Account("m/44'/1729'/0'/0'",
                SIGNATURE_TYPE.SECP256K1,
                "sppk7bVy617DmGvXsMqcwsiLtnedTN2trUi5ugXcNig7en4rHJyunK1"),
        Account("m/44'/1729'/0'/0'",
                SIGNATURE_TYPE.SECP256R1,
                "p2pk67fq5pzuMMABZ9RDrooYbLrgmnQbLt8z7PTGM9mskf7LXS5tdBG"),
        Account("m/44'/1729'/0'/0'",
                SIGNATURE_TYPE.BIP32_ED25519,
                "edpkumJgSsSxkpiB5hmTq6eZcrmc6BsJtLAhYceFTiziFqje4mongz")
    ]

    for account in accounts:

        app.assert_screen(Screen.Home)

        data = app.backend.get_public_key(account, with_prompt=False)

        app.checker.check_public_key(account, data)

    app.quit()
