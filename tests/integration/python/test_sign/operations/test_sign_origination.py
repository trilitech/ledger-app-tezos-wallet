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

"""Gathering of tests related to Origination operations."""

from utils.message import Default, Origination
from .helper import Flow, Field, TestOperation, pytest_generate_tests


class TestOrigination(TestOperation):
    """Commun tests."""

    @property
    def op_class(self):
        return Origination

    flows = [Flow('basic', delegate=Default.ED25519_PUBLIC_KEY_HASH)]

    fields = [
        Field("balance", "Balance", [
            Field.Case(0, "0"),
            Field.Case(1000, "1000"),
            Field.Case(1000000, "1000000"),
            Field.Case(1000000000, "1000000000"),
            Field.Case(0xFFFFFFFFFFFFFFFF, "max"),  # max uint64
        ]),
        Field("delegate", "Delegate", [
            Field.Case(None, "none"),
            Field.Case('tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa', "tz1"),
            Field.Case('tz2CJBeWWLsUDjVUDqGZL6od3DeBCNzYXrXk', "tz2"),
            Field.Case('tz3fLwHKthqhTPK6Lar6CTXN1WbDETw1YpGB', "tz3"),
            Field.Case('tz4AcerThk5nGtWNBiSqJfZFeWtz6ZqJ6mTY', "tz4"),
            Field.Case('tz1Kp8NCAN5WWwvkWkMmQQXMRe68iURmoQ8w', "long-hash"),
        ]),
        Field("code", "Code", [
            Field.Case([], "empty"),
            Field.Case([{'prim': 'CDR'}, {'prim': 'NIL', 'args': [{'prim': 'operation'}]}, {'prim': 'PAIR'}], "small"),
            # More test about Micheline in micheline tests
        ]),
        Field("storage", "Storage", [
            Field.Case({'prim': 'unit'}, "unit"),
            Field.Case({'prim': 'or', 'args': [{'prim': 'int'}, {'prim': 'string'}]}, "basic"),
            # More test about Micheline in micheline tests
        ]),
    ]
