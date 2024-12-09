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

from pathlib import Path
from typing import List, Optional

import pytest

from utils.account import Account
from utils.backend import TezosBackend
from utils.message import ScRollupOriginate
from utils.navigator import TezosNavigator


@pytest.mark.parametrize(
    "whitelist", [
        None,
        [],
        [
            'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
            'tz2WmivuMG8MMRKMEmzKRMMxMApxZQWYNS4W',
            'tz3XMQscBFM9vPmpbYMavMmwxRMUWvWGZMQQ',
        ],
    ],
    ids=[
            "no_whitelist",
            "empty_whitelist",
            "with_whitelist",
        ],
)
def test_sign_sc_rollup_originate(
        backend: TezosBackend,
        tezos_navigator: TezosNavigator,
        account: Account,
        whitelist: Optional[List[str]],
        snapshot_dir: Path
):
    """Check signing smart rollup originate"""

    tezos_navigator.toggle_expert_mode()

    message = ScRollupOriginate(
        source = 'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        fee = 10000,
        counter = 2,
        gas_limit = 3,
        storage_limit = 4,
        pvm_kind = "arith",
        kernel = '396630396632393532643334353238633733336639343631356366633339626335353536313966633535306464346136376261323230386365386538363761613364313361366566393964666265333263363937346161396132313530643231656361323963333334396535396331336239303831663163313162343430616334643334353564656462653465653064653135613861663632306434633836323437643964313332646531626236646132336435666639643864666664613232626139613834',
        parameters_ty = {'prim': 'Pair', 'args': [{'string': '1'}, {'int': 2}]},
        whitelist = whitelist
    )

    with backend.sign(account, message, with_hash=True) as result:
        tezos_navigator.accept_sign(snap_path=snapshot_dir)

    account.check_signature(
        message=message,
        with_hash=True,
        data=result.value
    )
