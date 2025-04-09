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

"""Gathering of tests related to Smart-rollup Originate operations."""

from utils.message import ScRollupOriginate
from .helper import Flow, Field, TestOperation, pytest_generate_tests


class TestScRollupOriginate(TestOperation):
    """Commun tests."""

    @property
    def op_class(self):
        return ScRollupOriginate

    flows = [
        Flow(
            'basic',
            kernel='0123456789ABCDEF',
            whitelist=['tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa']
        ),
        Flow('no-whitelist', whitelist=None),
        Flow('empty-whitelist', whitelist=[])
    ]

    fields = [
        Field("pvm_kind", "Kind", [
            Field.Case('arith', "arith"),
            Field.Case('wasm_2_0_0', "wasm_2_0_0"),
            Field.Case('riscv', "riscv"),
        ]),
        Field("kernel", "Kernel", [
            Field.Case('', 'empty'),
            Field.Case('0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF', 'long'),
        ]),
        Field("parameters_ty", "Parameters", [
            Field.Case({'prim': 'unit'}, "unit"),
            Field.Case({'prim': 'or', 'args': [{'prim': 'int'}, {'prim': 'string'}]}, "basic"),
            # More test about Micheline in micheline tests
        ]),
        Field("whitelist", "Whitelist", [
            Field.Case([
                'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
                'tz2W3Tvcm64GjcV2bipUynnEsctLFz5Z6yRa',
                'tz3XeTwXXJeWNgVR3LqMcyBDdnxjbZ7TeEGH',
                'tz4AcerThk5nGtWNBiSqJfZFeWtz6ZqJ6mTY',
                'tz1er74kx433vTtpYddGsf3dDt5piBZeeHyQ',
                'tz2JPgTWZZpxZZLqHMfS69UAy1UHm4Aw5iHu',
                'tz3fLwHKthqhTPK6Lar6CTXN1WbDETw1YpGB',
                'tz4J1Kjhjc3QpMxTaLeQaJxWR7DVV8VK5gdq',
                'tz1McCh72NRhYmJBcWr3zDrLJAxnfR9swcFh',
                'tz2CJBeWWLsUDjVUDqGZL6od3DeBCNzYXrXk',
                'tz3eydffbLkjdVb8zx42BvxpGV87zaRnqL3r',
                'tz4HQ7WgTRdgrxEdLWcGrgzYrHbz6a9ELZi3',
                'tz1TmFPVZsGQ8MnrBJtnECJgkFUwLa6EWYDm',
                'tz2KC42yW9FXFMJpkUooae2NFYQsM5do3E8H',
                'tz3hCsUiQDfneTgD7CSZDaUro8SA5aEhwCp2',
                'tz4H6NGpYd76yxZ4aGbPNKtWMJEEfZFBch2W',
                'tz1e8fEumaLvXXe5jV52gejCSt3mGodoKut9',
                'tz2PPZ2WN4j92Rdx4NM7oW3HAp3x825HUyac',
                'tz3Wazpbs4CFj78qv2KBJ8Z7HEyqk6ZPxwWZ',
                'tz4Mh4LFWMpACmKNWm1WNntMCPixsBWaMWMU',
            ], "many"),  # Max 4096
            Field.Case([
                'tz1Kp8NCAN5WWwvkWkMmQQXMRe68iURmoQ8w',
                'tz2WmivuMG8MMRKMEmzKRMMxMApxZQWYNS4W',
                'tz3XMQscBFM9vPmpbYMavMmwxRMUWvWGZMQQ',
                'tz4DNQhMQaU9WMCVGwH6mQGGWqMNQHTjywDe',
            ], "long-hash"),
        ]),
    ]
