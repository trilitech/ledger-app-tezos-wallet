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

"""Check signing smart rollup execute outbox message"""

from pathlib import Path

from utils.app import TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import ScRollupExecuteOutboxMessage

def test_sign_sc_rollup_execute_outbox_message(app: TezosAppScreen):
    """Check signing smart rollup execute outbox message"""
    test_name = Path(__file__).stem

    app.setup_expert_mode()

    message = ScRollupExecuteOutboxMessage(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 10000,
        counter = 2,
        gas_limit = 3,
        storage_limit = 4,
        rollup = 'sr163Lv22CdE8QagCwf48PWDTquk6isQwv57',
        cemented_commitment = 'src12UJzB8mg7yU6nWPzicH7ofJbFjyJEbHvwtZdfRXi8DQHNp1LY8',
        output_proof = b'9f09f2952d34528c733f94615cfc39bc555619fc550dd4a67ba2208ce8e867aa3d13a6ef99dfbe32c6974aa9a2150d21eca29c3349e59c13b9081f1c11b440ac4d3455dedbe4ee0de15a8af620d4c86247d9d132de1bb6da23d5ff9d8dffda22ba9a84'
    )

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
