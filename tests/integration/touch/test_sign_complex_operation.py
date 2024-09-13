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
    send_initialize_msg,
    send_payload,
    index_screen
)

## Operation (0): Origination
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 0.5 XTZ
# Storage limit: 4
# Balance: 1 XTZ
# Delegate: None
# Code: UNPACK mutez
# Storage: or key chest
## Operation (1): Transfer ticket
# Source: tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Fee: 0.01 XTZ
# Storage limit: 5
# Contents: None
# Type: option nat
# Ticketer: tz1TmFPVZsGQ8MnrBJtnECJgkFUwLa6EWYDm
# Amount: 7
# Destination: tz3eydffbLkjdVb8zx42BvxpGV87zaRnqL3r
# Entrypoint: default

if __name__ == "__main__":
    app = tezos_app(__file__)

    app.assert_home()

    send_initialize_msg(app, "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f81ffb40300000000000000000000000000000000000000000000000000000000000000006d00ffdd6102321bc251e4a5190ad5b12b251069d9b4a0c21e040304c0843d0000000004050d036a000000060764035c038d9e00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e05040500000002030600000004056303620000591e842444265757d6a65e3670ca18b5e662f9c0070002cc8e146741cf31fc00123b8c26baf95c57421a3c0000000764656661756c74")

    screen = "review"
    nb_screen = 2

    for index in range(1, nb_screen+1):
        app.review.next()
        app.assert_screen(index_screen(screen, index))

    app.review.next()
    app.expert_mode_splash()

    nb_screen_after = 4

    for index_after in range(index+1, index+nb_screen_after+1):
        app.review.next()
        app.assert_screen(index_screen(screen, index_after))

    app.review.next()
    app.assert_screen("operation_sign")

    expected_apdu = "5b8e95ffef018702781bef9aa935e05a879b79fa82f11d9cf067281144bb700237b3bcf1cd8222cbb9868341eabe58eddd212ed57c949cde5900444056a62cf049e61ff5acfe18a15166941810d283b52ff0a5b53b24416acb0f71643c925a0d9000"
    app.review_confirm_signing(expected_apdu)

    app.assert_home()
    app.quit()
