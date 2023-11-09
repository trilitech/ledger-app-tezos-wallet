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

# Expression: {"CACA";"POPO";"BOUDIN"}

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        account = Account("m/44'/1729'/0'/0'", SIGNATURE_TYPE.ED25519)
        data = app.sign_with_hash(account,
                                  "05020000001d0100000004434143410100000004504f504f0100000006424f5544494e",
                                  path=test_name)

        app.check_signature_with_hash(
            hash="84e475e38707140e725019e91f036e341fa4a2c8752b7828f37bbf91061b0e0a",
            signature="e0722bd72d15319474dff2207c137e85d57e742b7e5ccd1a995a610b8e055ad164e7606a37163b1a81e9003dc9e306afd46c4e645bbb190cf6c456459587ed04",
            data=data)

        app.quit()
