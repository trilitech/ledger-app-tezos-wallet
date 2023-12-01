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

# Operation (0): Transaction
# Fee: 0.05 XTZ
# Storage limit: 45
# Amount: 0.24 XTZ
# Destination: KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT
# Entrypoint: do
# Parameter: CAR

if __name__ == "__main__":
    test_name = Path(__file__).stem
    seed = "around dignity equal spread between young lawsuit interest climb wide that panther rather mom snake scene ecology reunion ice illegal brush"
    with nano_app(seed) as app:

        account = Account("m/44'/1729'/0'/0'",
                          SIGNATURE_TYPE.ED25519,
                          "edpkupntwMyERpYniuK1GDWquPaPU1wYsQgMirJPLGmC4Y5dMUsQNo")

        message = Message.from_bytes("0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316")

        data = app.sign(account,
                        message,
                        with_hash=True,
                        path=test_name)

        app.check_signature(
            account=account,
            message=message,
            with_hash=True,
            data=data)

        app.quit()
