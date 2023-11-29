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

from utils.app import *
from utils.backend import *

# Expression: {"CACA";"POPO";"BOUDIN"}

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        data = app.sign(DEFAULT_ACCOUNT,
                        "05020000001d0100000004434143410100000004504f504f0100000006424f5544494e",
                        with_hash=False,
                        path=test_name)

        app.check_signature(
            signature="e0722bd72d15319474dff2207c137e85d57e742b7e5ccd1a995a610b8e055ad164e7606a37163b1a81e9003dc9e306afd46c4e645bbb190cf6c456459587ed04",
            data=data)

        app.quit()
