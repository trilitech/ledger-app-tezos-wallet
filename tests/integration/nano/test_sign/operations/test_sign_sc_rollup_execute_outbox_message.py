#!/usr/bin/env python3
# Copyright 2024 Functori <contact@functori.com>
# Copyright 2024 Trilitech <contact@trili.tech>

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Gathering of tests related to Smart-rollup Execute-outbox operations."""

from utils.message import ScRollupExecuteOutboxMessage
from .helper import Flow, Field, TestOperation, pytest_generate_tests


class TestScRollupExecuteOutboxMessage(TestOperation):
    """Commun tests."""

    @property
    def op_class(self):
        return ScRollupExecuteOutboxMessage

    flows = [Flow('basic', output_proof=b'0123456789ABCDEF')]

    fields = [
        Field("rollup", "Rollup", [
            Field.Case('sr163Lv22CdE8QagCwf48PWDTquk6isQwv57', "sr1"),
            Field.Case('sr1M9PWZMtqu3MW2dMNCJNo3NApM2Wx9DwMW', "long-hash"),
        ]),
        Field("cemented_commitment", "Commitment", [
            Field.Case('src12UJzB8mg7yU6nWPzicH7ofJbFjyJEbHvwtZdfRXi8DQHNp1LY8', "src1"),
            Field.Case('src13w9G1om8PMHmgMNEwX6NMWKAgDSfmU4mZXbkNWbikmCuWhuB7W', "long-hash"),
        ]),
        Field("output_proof", "Output proof", [
            Field.Case(b'', "empty"),
            Field.Case(b'0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF', "long"),
        ]),
    ]
