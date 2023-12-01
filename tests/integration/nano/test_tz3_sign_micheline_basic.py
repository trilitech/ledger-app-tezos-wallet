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

from pathlib import Path

from utils.account import Account, SIGNATURE_TYPE
from utils.app import nano_app
from utils.message import Message

# Expression: {"CACA";"POPO";"BOUDIN"}

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        account = Account("m/44'/1729'/0'/0'",
                          SIGNATURE_TYPE.SECP256R1,
                          "p2pk67fq5pzuMMABZ9RDrooYbLrgmnQbLt8z7PTGM9mskf7LXS5tdBG")

        message = Message.from_bytes("05020000001d0100000004434143410100000004504f504f0100000006424f5544494e")

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
