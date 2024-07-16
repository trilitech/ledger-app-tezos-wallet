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
    verify_reject_response,
    index_screen
)

# full input: 0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316
# full output: CAR
# signer: tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E
# path: m/44'/1729'/0'/0'


def sign_transfer_initialize(app):
    app.assert_home()
    send_initialize_msg(app,"800f000011048000002c800006c18000000080000000")
    send_payload(app,"800f81005e0300000000000000000000000000000000000000000000000000000000000000006c016e8874874d31c3fbd636e924d5a036a43ec8faa7d0860308362d80d30e01000000000000000000000000000000000000000000ff02000000020316")

    screen = "tst_review"
    nb_screen = 2

    for index in range(1, nb_screen+1):
        app.review.next()
        app.assert_screen(index_screen(screen, index))


if __name__ == "__main__":
    app = tezos_app(__file__)
    #  Reject from enable expert mode
    sign_transfer_initialize(app)
    app.review.next()
    verify_reject_response(app,"enable_expert_mode")

    # Reject from expert mode splash
    sign_transfer_initialize(app)
    app.review.next()
    app.enable_expert_mode()
    verify_reject_response(app, "expert_mode_splash")

    # Now with expert mode enabled, reject from splash screen.

    sign_transfer_initialize(app)
    app.review.next()
    verify_reject_response(app, "expert_mode_splash")
