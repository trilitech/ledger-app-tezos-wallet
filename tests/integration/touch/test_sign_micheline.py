#!/usr/bin/env python3
# Copyright 2023 Trilitech <contact@trili.tech>

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from utils import (
    tezos_app,
    send_payload
)

# full input: 0502000000080100000001300000
# full output: Expression {"0";0}

if __name__ == "__main__":
    app = tezos_app(__file__)

    app.assert_home()

    app.send_initialize_msg("8004000011048000002c800006c18000000080000000")
    send_payload(app, "800481ff0e0502000000080100000001300000")

    app.review.next()
    app.assert_screen("micheline_screen")

    expected_apdu = "c871155802f26f35ea07a15c8002d836a8779ac8cc2feb0141f08831dad0326a0905cd4fc8f00615b7b4a78e7eaf7fcc0b7959e01e3119b6261ebd71d76d580b9000"
    app.review_confirm_signing(expected_apdu)

    app.assert_home()
    app.quit()
