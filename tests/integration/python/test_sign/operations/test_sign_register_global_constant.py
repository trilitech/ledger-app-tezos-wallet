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

"""Gathering of tests related to Register-global-constant operations."""

from utils.message import Default, RegisterGlobalConstant
from .helper import Flow, Field, TestOperation, pytest_generate_tests


class TestRegisterGlobalConstant(TestOperation):
    """Commun tests."""

    @property
    def op_class(self):
        return RegisterGlobalConstant

    flows = [Flow('basic')]

    fields = [
        Field("value", "Value", [
            Field.Case({'prim': 'Unit'}, "unit"),
            Field.Case({'prim': 'Pair', 'args': [{'string': 'a'}, {'int': 1}]}, "basic"),
            Field.Case({"prim": "constant", 'args': [{"string": Default.SCRIPT_EXPR_HASH}]}, "with-constant")
            # More test about Micheline in micheline tests
        ]),
    ]
