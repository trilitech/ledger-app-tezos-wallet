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

"""Check reject pk behaviour"""

from pathlib import Path

from utils.app import Screen, TezosAppScreen, DEFAULT_ACCOUNT

def test_reject_pk(app: TezosAppScreen):
    """Check reject pk behaviour"""
    test_name = Path(__file__).stem

    app.assert_screen(Screen.HOME)

    app.reject_public_key(DEFAULT_ACCOUNT, test_name)

    app.quit()
