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

"""Check signing transaction with a long destination hash"""

from pathlib import Path

from utils.app import TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import RawMessage

# Operation (0): Transaction
# Fee: 0.01 XTZ
# Storage limit: 4
# Amount: 0 XTZ
# Destination: KT1GW4QHn66m7WWWMWCMNaWmGYpCRbg5ahwU
# Entrypoint: root
# Parameter: 0

def test_sign_with_long_hash(app: TezosAppScreen):
    """Check signing transaction with a long destination hash"""
    test_name = Path(__file__).stem

    app.setup_expert_mode()

    message = RawMessage("0300000000000000000000000000000000000000000000000000000000000000006c00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e020304000156dcfb211fa76c525fd7c4566c09a5e3e4d5b81000ff01000000020000")

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
