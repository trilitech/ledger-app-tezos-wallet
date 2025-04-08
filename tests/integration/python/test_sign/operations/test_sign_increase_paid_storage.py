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

"""Gathering of tests related to Increase-paid-storage operations."""

from utils.message import IncreasePaidStorage
from .helper import Flow, Field, TestOperation, pytest_generate_tests


class TestIncreasePaidStorage(TestOperation):
    """Commun tests."""

    @property
    def op_class(self):
        return IncreasePaidStorage

    flows = [Flow('basic')]

    fields = [
        Field("amount", "Amount", [
            Field.Case(0, "0"),
            Field.Case(1000, "1000"),
            Field.Case(1000000, "1000000"),
            Field.Case(1000000000, "1000000000"),
            Field.Case(0xFFFFFFFFFFFFFFFF, "max"),  # max uint64
        ]),
        Field("destination", "Destination", [
            Field.Case('KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT', "kt1"),
            Field.Case('KT1GW4QHn66m7WWWMWCMNaWmGYpCRbg5ahwU', "long-hash"),
        ]),
    ]
