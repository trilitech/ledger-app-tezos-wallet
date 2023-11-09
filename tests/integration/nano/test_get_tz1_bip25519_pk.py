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
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        account = Account("m/44'/1729'/0'/0'", SIGNATURE_TYPE.BIP32_ED25519)
        data = app.backend.get_public_key(account)

        app.check_public_key(
            public_key="210293c6b359964a4332bf1355579d665b753343f7b0a42567978cea1671f7b89f47",
            data=data)

        app.quit()