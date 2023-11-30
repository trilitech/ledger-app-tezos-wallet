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

# Operation (0): SR: send messages
# Fee: 0.01 XTZ
# Storage limit: 4
# Message (0): 012345
# Message (1): 67
# Message (2): 89abcdef

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        message = "030000000000000000000000000000000000000000000000000000000000000000c900ffdd6102321bc251e4a5190ad5b12b251069d9b4904e020304000000140000000301234500000001670000000489abcdef"

        data = app.sign(DEFAULT_ACCOUNT,
                        message,
                        with_hash=True,
                        path=test_name)

        app.check_signature_with_hash(
            account=DEFAULT_ACCOUNT,
            message=message,
            hash="5ec1f51c235fecb7e66dd35acc31bf31a6fbc2aae1716ada0b953f3a95b91b6b",
            data=data)

        app.quit()
