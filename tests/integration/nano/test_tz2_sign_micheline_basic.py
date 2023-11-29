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

from utils.app import *
from utils.backend import *

# Expression: {"CACA";"POPO";"BOUDIN"}

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        account = Account("m/44'/1729'/0'/0'",
                          SIGNATURE_TYPE.SECP256K1,
                          "sppk7bVy617DmGvXsMqcwsiLtnedTN2trUi5ugXcNig7en4rHJyunK1")
        message="05020000001d0100000004434143410100000004504f504f0100000006424f5544494e"
        data = app.sign(account,
                        message,
                        with_hash=True,
                        path=test_name)

        app.check_tlv_signature_with_hash(
            hash="84e475e38707140e725019e91f036e341fa4a2c8752b7828f37bbf91061b0e0a",
            message=message,
            pk="sppk7bVy617DmGvXsMqcwsiLtnedTN2trUi5ugXcNig7en4rHJyunK1",
            data=data)

        app.quit()
