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

# Operation (0): Transaction
# Fee: 0.05 tz
# Storage limit: 45
# Amount: 0.24 tz
# Destination: KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT
# Entrypoint: do
# Parameter: CAR

if __name__ == "__main__":
    test_name = Path(__file__).stem
    seed = "around dignity equal spread between young lawsuit interest climb wide that panther rather mom snake scene ecology reunion ice illegal brush"
    with nano_app(seed) as app:

        data = app.sign_with_hash(DEFAULT_ACCOUNT,
                                  "0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316",
                                  path=test_name)

        app.check_signature_with_hash(
            hash="f6d5fa0e79cac216e25104938ac873ca17ee9d7f06763719293b413cf2ed475c",
            signature="7365f6549fa50c31591f348efe2684982a038fe44458077e72d5cf7b4284d887f72a8e3842f9595fee7176c6062cdfdfae132c9330a77680958c652731e99a0e",
            data=data)

        app.quit()
