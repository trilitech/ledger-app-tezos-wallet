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

        account = Account("m/44'/1729'/0'/0'", SIGNATURE_TYPE.ED25519)
        data = app.provide_public_key(account, test_name)

        app.check_public_key(
            public_key="2102747884d9abdf16b3ab745158925f567e222f71225501826fa83347f6cbe9c393",
            data=data)

        app.quit()
