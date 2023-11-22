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

# Operation (0): Set consensus key
# Fee: 0.01 XTZ
# Storage limit: 4
# Public key: edpkuXX2VdkdXzkN11oLCb8Aurdo1BTAtQiK8ZY9UPj2YMt3AHEpcY

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        data = app.sign_with_hash(DEFAULT_ACCOUNT,
                                  "0300000000000000000000000000000000000000000000000000000000000000007200c921d4487c90b4472da6cc566a58d79f0d991dbf904e02030400747884d9abdf16b3ab745158925f567e222f71225501826fa83347f6cbe9c393",
                                  path=test_name)

        app.check_signature_with_hash(
            hash="f9570b3272e25bc3a9c17a489547dcae70ae750adaa73b4b8eb5dd0b55be5987",
            signature="dd9c53607303e7c5f9c11488d5c8977d79e248f59f753eabf5ab7babfa61b1f6279b2e16dacb6db87e4e05cbdc23156d3e95989161d322ba2feb369beeb7b504",
            data=data)

        app.quit()
