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

        app.assert_screen(Screen.Home)

        account = Account("m/44'/1729'/0'/0'", SIGNATURE_TYPE.SECP256R1)
        data = app.provide_public_key(account, test_name)

        app.check_public_key(
            public_key="410497f4d381101d2908a13669313faec5dbf6693985584f96268ea2c25178199ddd1aad041e7564795eb4b9a4f379e8cdc0c8391f7b2880613771fff76e6a6b05cf",
            data=data)

        app.quit()
