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
from .helper import Flow, Field, TestOperation, pytest_generate_tests


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

    fields = [
        Field("amount", "Amount", [
            Field.Case(0, "0"),
            Field.Case(1000, "1000"),
            Field.Case(1000000, "1000000"),
            Field.Case(1000000000, "1000000000"),
            Field.Case(0xFFFFFFFFFFFFFFFF, "max"),  # max uint64
        ]),
        Field("destination", "Destination", [
            Field.Case('tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa', "tz1"),
            Field.Case('tz2CJBeWWLsUDjVUDqGZL6od3DeBCNzYXrXk', "tz2"),
            Field.Case('tz3fLwHKthqhTPK6Lar6CTXN1WbDETw1YpGB', "tz3"),
            Field.Case('KT18amZmM5W7qDWVt2pH6uj7sCEd3kbzLrHT', "kt1"),
            Field.Case('tz1Kp8NCAN5WWwvkWkMmQQXMRe68iURmoQ8w', "long-hash"),
        ]),
        Field("entrypoint", "Entrypoint", [
            # `parameter` is set to make sure `entrypoint` is displayed with `default`
            Field.Case('default', "default", parameter={'prim': 'None'}),
            Field.Case('root', "root"),
            Field.Case('do', "do"),
            Field.Case('set_delegate', "set_delegate"),
            Field.Case('remove_delegate', "remove_delegate"),
            Field.Case('deposit', "deposit"),
            Field.Case('stake', "stake"),
            Field.Case('unstake', "unstake"),
            Field.Case('finalize_unstake', "finalize_unstake"),
            Field.Case('set_delegate_parameters', "set_delegate_parameters"),
            Field.Case('custom_entrypoint', "custom_entrypoint"),
        ]),
        Field("parameter", "Parameter", [
            # `entrypoint` is set to make sure `parameter` is displayed with `unit`
            Field.Case({'prim': 'Unit'}, "unit", entrypoint='entrypoint'),
            Field.Case({'prim': 'Pair', 'args': [{'string': 'a'}, {'int': 1}]}, "basic"),
            # More test about Micheline in micheline tests
        ]),
    ]
