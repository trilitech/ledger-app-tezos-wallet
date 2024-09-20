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

from utils import (
    tezos_app,

    send_payload,
    index_screen
)

# full input: 0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316
# full output: CAR
# signer: tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E
# path: m/44'/1729'/0'/0'

if __name__ == "__main__":
    app = tezos_app(__file__)

    app.assert_home()

    app.send_initialize_msg( "8004000011048000002c800006c18000000080000000")
    send_payload(app, "800481005e0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316")

    screen = "review"
    nb_screen = 2

    for index in range(1, nb_screen+1):
        app.review.next()
        app.assert_screen(index_screen(screen, index))

    app.review.next()
    app.expert_mode_splash()

    app.review.next()
    app.assert_screen(index_screen(screen, index+1))

    expected_apdu = "f63d045a1cc9f73eee5775c5d496fa9d3aa9ae57fb97217f746a8728639795b7b2220e84ce5759ed111399ea3263d810c230d6a4fffcb6e82797c5ca673a17089000"
    app.review_confirm_signing(expected_apdu)

    app.assert_home()
    app.quit()
