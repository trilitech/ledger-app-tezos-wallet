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

from utils.apdu import *
from utils.app import *

if __name__ == "__main__":
    test_name = Path(__file__).stem

    def make_path(name: str) -> Path:
        return Path(test_name) / name

    with nano_app() as app:

        app.assert_screen(Screen.Home)

        app.reject_public_key(DEFAULT_ACCOUNT, make_path("reject_public_key"))

        app.assert_screen(Screen.Home)

        app.reject_signing(DEFAULT_ACCOUNT,
                           "0300000000000000000000000000000000000000000000000000000000000000006c00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e0203040001000000000000000000000000000000000000000000ff01000001070200000102076501000000015b076501000000015a0765010000000159076501000000015807650100000001570765010000000156076501000000015507650100000001540765010000000153076501000000015207650100000001510765010000000150076501000000014f076501000000014e076501000000014d076501000000014c076501000000014b076501000000014a0765010000000149076501000000014807650100000001470765010000000146076501000000014507650100000001440765010000000143076501000000014202000000000765000a0765000907650008076500070765000607650005076500040765000307650002076500010200000000",
                           with_hash=True,
                           path=make_path("reject_signing"))

        data = app.backend.get_public_key(DEFAULT_ACCOUNT, with_prompt=False)

        app.check_public_key(DEFAULT_ACCOUNT, data)

        app.quit()
