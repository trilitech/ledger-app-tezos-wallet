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

import os
import sys

file_path=os.path.abspath(__file__)
dir_path=os.path.dirname(file_path)
root_path=os.path.dirname(dir_path)
sys.path.append(root_path)

from conftest import requires_device

from pathlib import Path

from utils.app import Screen, DEFAULT_ACCOUNT
from utils.message import Message

# Operation (0): Transaction
# Fee: 0.39 XTZ
# Storage limit: 6
# Amount: 0.02 XTZ
# Destination: tz1cfdVKpBb9VRBdny8BQ5RSK82UudAp2miM
# Entrypoint: jean_bob
# Parameter: {Pair {} (Right -76723569314251090535296646);Pair {Elt Unit (Pair {Left Unit} (Pair (Left 0x03F01167865DC63DFEE0E31251329CEAB660D94606) (Pair 0x0107B21FCA96C5763F67B286752C7AAEFC5931D15A Unit)))} (Right 3120123370638446806591421154959427514880865200209654970345);Pair {} (Left (Some Unit))}
# Operation (1): Transaction
# Fee: 0.65 XTZ
# Storage limit: 2
# Amount: 0.06 XTZ
# Destination: KT1CYT8oACUcCSNTu2qfgB4fj5bD7szYrpti

@requires_device("nanos")
def test_nanos_regression_batched_ops(app):
    test_name = Path(__file__).stem

    app.setup_expert_mode()
    app.setup_blindsign_off()

    message = Message.from_bytes("0300000000000000000000000000000000000000000000000000000000000000006c001597c45b11b421bb806a0c56c5da5638bf4b1adbf0e617090006a09c010000bac799dfc7f6af2ff0b95f83d023e68c895020baffff086a65616e5f626f620000009a020000009507070200000000050800c6bab5ccc8d891cd8de4b6f7070707020000004b0704030b070702000000040505030b070705050a0000001503f01167865dc63dfee0e31251329ceab660d9460607070a000000150107b21fca96c5763f67b286752c7aaefc5931d15a030b050800a9df9fc1e7eaa7a9c1f7bd87a9ba9cadf5b5b2cd829deea2b7fef9070707020000000005050509030b6c01ee572f02e5be5d097ba17369789582882e8abb8790d627063202e0d403012b704944f5b5fd30eed2ab4385478488e09fe04a0000")

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
