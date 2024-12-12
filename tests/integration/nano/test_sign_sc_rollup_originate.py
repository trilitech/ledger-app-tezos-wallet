#!/usr/bin/env python3
# Copyright 2023 Functori <contact@functori.com>

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Check signing smart rollup originate"""

from pathlib import Path
from typing import List, Optional

from utils.app import TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import ScRollupOriginate

def test_sign_sc_rollup_originate(app: TezosAppScreen):
    """Check signing smart rollup originate"""
    test_name = Path(__file__).stem

    app.setup_expert_mode()

    def check_sign(name: str, whitelist: Optional[List[str]]):

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

        data = app.sign(DEFAULT_ACCOUNT,
                        message,
                        with_hash=True,
                        path=Path(test_name) / name)

        app.checker.check_signature(
            account=DEFAULT_ACCOUNT,
            message=message,
            with_hash=True,
            data=data)

    check_sign("no_whitelist", None)
    check_sign("no_whitelist", [])
    check_sign("with_whitelist", [
        'tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa',
        'tz2WmivuMG8MMRKMEmzKRMMxMApxZQWYNS4W',
        'tz3XMQscBFM9vPmpbYMavMmwxRMUWvWGZMQQ'
    ])

    app.quit()
