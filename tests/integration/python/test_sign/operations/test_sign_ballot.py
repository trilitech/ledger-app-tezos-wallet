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

"""Gathering of tests related to Ballot operations."""

from utils.message import Ballot
from .helper import Flow, Field, TestOperation, pytest_generate_tests


class TestBallot(TestOperation):
    """Commun tests."""

    @property
    def op_class(self):
        return Ballot

    flows = [Flow('basic')]

    fields = [
        Field("source", "Source", [
            Field.Case('tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa', "tz1"),
            Field.Case('tz2CJBeWWLsUDjVUDqGZL6od3DeBCNzYXrXk', "tz2"),
            Field.Case('tz3fLwHKthqhTPK6Lar6CTXN1WbDETw1YpGB', "tz3"),
            Field.Case('tz1Kp8NCAN5WWwvkWkMmQQXMRe68iURmoQ8w', "long-hash"),
        ]),
        Field("period", "Period", [
            Field.Case(0, "0"),
            Field.Case(32, "32"),
            Field.Case(0x7FFFFFFF, "max"),  # max int32
            Field.Case(0x80000000, "min"),  # min int32
        ]),
        Field("proposal", "Proposal", [
            Field.Case('PsParisCZo7KAh1Z1smVd9ZMZ1HHn5gkzbM94V3PLCpknFWhUAi', "basic"),
            Field.Case('Ptd4kYMModZQ6Mh4ZRNMmWpM799PgSzjmGw3GM9Q2SDqqo8WCW8', "long-hash"),
        ]),
        Field("ballot", "Ballot", [
            Field.Case('yay', "yay"),
            Field.Case('nay', "nay"),
            Field.Case('pass', "pass"),
        ]),
    ]
