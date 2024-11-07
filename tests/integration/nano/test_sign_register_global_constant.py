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

"""Check signing register global constant"""

from pathlib import Path

from utils.app import TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import Message

# Operation (0): Register global constant
# Fee: 0.01 XTZ
# Storage limit: 4
# Value: Pair "1" 2

def test_sign_register_global_constant(app: TezosAppScreen):
    """Check signing register global constant"""
    test_name = Path(__file__).stem

    app.setup_expert_mode()

    message = Message.from_bytes("0300000000000000000000000000000000000000000000000000000000000000006f00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e0203040000000a07070100000001310002")

    data = app.sign(DEFAULT_ACCOUNT,
                    message,
                    with_hash=True,
                    path=test_name)

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=message,
        with_hash=True,
        data=data)

    app.quit()
