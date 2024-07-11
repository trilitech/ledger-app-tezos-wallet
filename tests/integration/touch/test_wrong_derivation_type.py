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

from utils import tezos_app

if __name__ == "__main__":
    app = tezos_app(__file__)

    app.assert_home()

    # INS_GET_PUBLIC_KEY
    app.send_apdu("8002000411048000002c800006c18000000080000000")
    app.expect_apdu_failure("6b00")

    # INS_PROMPT_PUBLIC_KEY
    app.send_apdu("8003000411048000002c800006c18000000080000000")
    app.expect_apdu_failure("6b00")

    # INS_SIGN
    app.send_apdu("8004000411048000002c800006c18000000080000000")
    app.expect_apdu_failure("6b00")

    # INS_SIGN_WITH_HASH
    app.send_apdu("800f000411048000002c800006c18000000080000000")
    app.expect_apdu_failure("6b00")

    app.assert_home()
    app.quit()
