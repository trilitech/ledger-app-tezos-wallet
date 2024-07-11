#!/usr/bin/env python3
# Copyright 2023 Functori <contact@functori.com>
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

from utils import *

# full input: 0000000000000000000000000000000000000000000000000000000000000000ce00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e02030400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000c6396630396632393532643334353238633733336639343631356366633339626335353536313966633535306464346136376261323230386365386538363761613364313361366566393964666265333263363937346161396132313530643231656361323963333334396535396331336239303831663163313162343430616334643334353564656462653465653064653135613861663632306434633836323437643964313332646531626236646132336435666639643864666664613232626139613834
# signer: tz1dyX3B1CFYa2DfdFLyPtiJCfQRUgPVME6E

if __name__ == "__main__":
    app = stax_app(__file__)

    app.assert_home()

    send_initialize_msg(app, "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f0100eb030000000000000000000000000000000000000000000000000000000000000000ce00ffdd6102321bc251e4a5190ad5b12b251069d9b4904e02030400000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000c63966303966323935326433343532386337333366393436313563666333396263353535363139666335353064643461363762613232303863653865383637616133643133613665663939646662653332633639373461613961323135306432316563613239633333343965353963313362393038316631")
    app.review.tap()
    app.assert_screen("tseom_review_00")

    app.review.tap()
    app.assert_screen("tseom_review_01")
    app.review.tap()
    app.expect_apdu_return("9000")

    app.send_apdu("800f82004f63313162343430616334643334353564656462653465653064653135613861663632306434633836323437643964313332646531626236646132336435666639643864666664613232626139613834")

    app.assert_screen("tseom_review_02")
    app.review.tap()
    app.expert_mode_splash()


    for i in range(4):
        app.review.tap()
        app.assert_screen(f"operation_proof_396...834_{i}")

    app.review.tap()
    app.assert_screen("operation_sign")

    expected_apdu = "c08f5e1a02d15b05c4066b43fc31aa1ccad30f6c7a18f44723e5af0b6584292236e919219e90793ef502e8883f5317206277607438695933fcb954f4ef451db19628a114880836193c755ddda4bf188b9764231975b2c5ecb64bc4bdc9c459039000"
    app.review_confirm_signing(expected_apdu)

    app.assert_home()
    app.quit()
