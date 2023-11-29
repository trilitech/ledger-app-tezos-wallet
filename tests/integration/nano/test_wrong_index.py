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

if __name__ == "__main__":
    with nano_app() as app:
        for ins in [INS.GET_PUBLIC_KEY,
                    INS.PROMPT_PUBLIC_KEY]:
            for index in [INDEX.OTHER,
                          INDEX.LAST]:

                app.assert_screen(Screen.Home)

                with app.expect_apdu_failure(StatusCode.WRONG_PARAM):
                    app.backend._exchange(ins,
                                          index=index,
                                          sig_type=DEFAULT_ACCOUNT.sig_type,
                                          payload=DEFAULT_ACCOUNT.path)

        app.quit()
