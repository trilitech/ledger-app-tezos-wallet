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

"""Gathering of tests related to Smart-rollup Add-message operations."""

from utils.message import ScRollupAddMessage
from .helper import Flow, Field, TestOperation, pytest_generate_tests


class TestScRollupAddMessage(TestOperation):
    """Commun tests."""

    @property
    def op_class(self):
        return ScRollupAddMessage

    flows = [
        Flow('basic', message=[bytes.fromhex('0123456789ABCDEF')]),
        Flow('none', message=[])
    ]

    fields = [
        Field("message", "Message", [
            Field.Case([b''], "empty"),
            Field.Case([bytes.fromhex('0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF')], "long"),
            Field.Case([b'\00'] * 20, "many"),  # No max
        ]),
    ]
