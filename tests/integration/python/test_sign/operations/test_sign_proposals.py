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

"""Gathering of tests related to Proposals operations."""

from utils.message import Proposals
from .helper import Flow, Field, TestOperation, pytest_generate_tests


class TestProposals(TestOperation):
    """Commun tests."""

    @property
    def op_class(self):
        return Proposals

    flows = [
        Flow('basic'),
        Flow('empty-proposals', proposals=[])
    ]

    fields = [
        Field("source", "Source", [
            Field.Case('tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa', "tz1"),
            Field.Case('tz2CJBeWWLsUDjVUDqGZL6od3DeBCNzYXrXk', "tz2"),
            Field.Case('tz3fLwHKthqhTPK6Lar6CTXN1WbDETw1YpGB', "tz3"),
            Field.Case('tz4AcerThk5nGtWNBiSqJfZFeWtz6ZqJ6mTY', "tz4"),
            Field.Case('tz1Kp8NCAN5WWwvkWkMmQQXMRe68iURmoQ8w', "long-hash"),
        ]),
        Field("period", "Period", [
            Field.Case(0, "0"),
            Field.Case(32, "32"),
            Field.Case(0x7FFFFFFF, "max"),  # max int32
            Field.Case(0x80000000, "min"),  # min int32
        ]),
        Field("proposals", "Proposal", [
            Field.Case([
                'Ps9mPmXaRzmzk35gbAYNCAw6UXdE2qoABTHbN2oEEc1qM7CwT9P',
                'PtCJ7pwoxe8JasnHY8YonnLYjcVHmhiARPJvqcC6VfHT5s8k8sY',
                'PsYLVpVvgbLhAhoqAkMFUo6gudkJ9weNXhUYCiLDzcUpFpkk8Wt',
                'PsddFKi32cMJ2qPjf43Qv5GDWLDPZb3T3bF6fLKiF5HtvHNU7aP',
                'Pt24m4xiPbLDhVgVfABUjirbmda3yohdN82Sp9FeuAXJ4eV9otd',
                'PsBABY5HQTSkA4297zNHfsZNKtxULfL18y95qb3m53QJiXGmrbU',
                'PsBabyM1eUXZseaJdmXFApDSBqj8YBfwELoxZHHW77EMcAbbwAS',
                'PsCARTHAGazKbHtnKfLzQg3kms52kSRpgnDY982a9oYsSXRLQEb',
                'PsDELPH1Kxsxt8f9eWbxQeRxkjfbxoqM52jvs5Y5fBxWWh4ifpo',
                'PtEdoTezd3RHSC31mpxxo1npxFjoWWcFgQtxapi51Z8TLu6v6Uq',
                'PtEdo2ZkT9oKpimTah6x2embF25oss54njMuPzkJTEi5RqfdZFA',
                'PsFLorenaUUuikDWvMDr6fGBRG8kt3e3D3fHoXK1j1BFRxeSH4i',
                'PtGRANADsDU8R9daYKAgWnQYAJ64omN1o3KMGVCykShA97vQbvV',
                'PtHangz2aRngywmSRGGvrcTyMbbdpWdpFKuS4uMWxg2RaH9i1qx',
                'Psithaca2MLRFYargivpo7YvUr7wUDqyxrdhC5CQq78mRvimz6A',
                'PtJakart2xVj7pYXJBXrqHgd82rdkLey5ZeeGwDgPp9rhQUbSqY',
                'PtKathmankSpLLDALzWw7CGD2j2MtyveTwboEYokqUCP4a1LxMg',
                'PtLimaPtLMwfNinJi9rCfDPWea8dFgTZ1MeJ9f1m2SRic6ayiwW',
                'PtMumbai2TmsJHNGRkD8v8YDbtao7BLUC3wjASn1inAKLFCjaH1',
                'PtNairobiyssHuh87hEhfVBGCVrK3WnS8Z2FT4ymB5tAa4r1nQf',
            ], "full"),  # 20 max
            Field.Case(['Ptd4kYMModZQ6Mh4ZRNMmWpM799PgSzjmGw3GM9Q2SDqqo8WCW8'], "long-hash"),
        ]),
    ]
