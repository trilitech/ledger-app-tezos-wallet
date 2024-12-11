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

"""Gathering of tests related to Transaction operations."""

from utils.message import Transaction
from .helper import Flow, TestOperation, pytest_generate_tests


class TestTransaction(TestOperation):
    """Commun tests."""

    @property
    def op_class(self):
        return Transaction

    flows = [
        Flow('basic', amount=5),
        Flow(
            'contract_call',
            destination='KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT',
            entrypoint="transfer",
            parameter=[
                {'prim': 'Pair', 'args': [
                    {'string': 'KT1QWdbASvaTXW8GWfhfNh3JMjgXvnZAATJW'},
                    [{'prim': 'Pair', 'args': [
                        {'string': 'sr1MyCwR83hZphCSqaYSQApPxPMeyksJWWnh'},
                        [{'prim': 'Pair', 'args': [
                            {'int': 0},
                            {'int': 5432900665191893635}
                        ]}]
                    ]}]
                ]}
            ]
        ),
        Flow('stake', amount=1000000000, entrypoint='stake'),
        Flow('unstake', amount=500000000, entrypoint='unstake'),
        Flow('finalize_unstake', entrypoint='finalize_unstake'),
        Flow(
            'delegate_parameters',
            entrypoint='delegate_parameters',
            parameter={'prim': 'Pair', 'args': [
                {'int': 4000000},
                {'prim': 'Pair', 'args': [
                    {'int': 20000000},
                    {'prim': 'Unit'}
                ]}
            ]}
        ),
    ]
