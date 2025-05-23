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

"""Gathering of tests related to Set-deposit-limit operations."""

from utils.message import SetDepositLimit
from .helper import Flow, Field, TestOperation, pytest_generate_tests


class TestSetDepositLimit(TestOperation):
    """Commun tests."""

    @property
    def op_class(self):
        return SetDepositLimit

    flows = [Flow('basic', limit=400)]

    fields = [
        Field("limit", "Staking limit", [
            Field.Case(None, "none"),
            Field.Case(0, "0"),
            Field.Case(1000, "1000"),
            Field.Case(1000000, "1000000"),
            Field.Case(1000000000, "1000000000"),
            Field.Case(0xFFFFFFFFFFFFFFFF, "max"),  # max uint64
        ]),
    ]
