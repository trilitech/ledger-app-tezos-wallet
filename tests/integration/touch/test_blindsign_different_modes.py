#!/usr/bin/env python3
# Copyright 2023 Trilitech <contact@trili.tech>
# Copyright 2024 Functori <contact@functori.com>

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
    TezosAppScreen,
    send_payload,
    BlindsigningStatus,
)

from ragger.firmware import Firmware
#Response:  c9fc57555a59876454427adadeb62cf365bf936e346def12f0729e6a1c9d0eed81e1acced76fddb6ec90619a12d8904dd9ba07f64a9f2c4e05a692224ec7bdb1d357b90a03a0d8f441b048d0cff72e997aac00d657725f67afb68c76eacb79029000

def navigate_screens(app: TezosAppScreen, group: int, screen_count: int, suffix: str):
    for i in range(screen_count):
        app.assert_screen("tbdm_screen_" + suffix + str(group) + "_" + str(i+1))
        app.review.next()


def navigate_common(app: TezosAppScreen, skip: bool =False, group_counts: list = []):
    suffix = ""
    if skip:
        suffix = "skip_"
    app.send_initialize_msg( "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f01ffeb0300000000000000000000000000000000000000000000000000000000000000006b00ffdd6102321bc251e4a5190ad5b12b251069d9b4c0843d0b0104020320182716513907b6bab33f905396d031931c07e01bddd780780c1a56b9c086da6c00ffdd6102321bc251e4a5190ad5b12b251069d9b480897a0c0107c08db701000278eb8b6ab9a768579cd5146b480789650c83f28effff0d7570646174655f636f6e6669670000000607070005030a6e00ffdd6102321bc251e4a5190ad5b12b251069d9b4c08db7010d0105ff01ee572f02e5be5d097ba17369789582882e8abb87c900ffdd6102321bc2")
    app.assert_screen("review_transaction")
    app.review.next()
    navigate_screens(app, group= 1, screen_count = group_counts[1 - 1 ], suffix=suffix)
    app.assert_screen("expert_mode_splash")
    app.review.next()
    navigate_screens(app, group= 2, screen_count= group_counts[2 - 1 ], suffix=suffix)
    app.expect_apdu_return("9000")
    app.send_apdu("800f01ffeb51e4a5190ad5b12b251069d9b48092f4010e0106000000fa000000086d65737361676530000000086d65737361676531000000086d65737361676532000000086d65737361676533000000086d65737361676534000000086d65737361676535000000086d65737361676536000000086d65737361676537000000086d65737361676538000000086d65737361676539000000096d6573736167653130000000096d6573736167653131000000096d6573736167653132000000096d6573736167653133000000096d6573736167653134000000096d6573736167653135000000096d6573736167653136")
    navigate_screens(app, group= 3, screen_count= group_counts[3 - 1 ], suffix=suffix)
    app.expect_apdu_return("9000")
    app.send_apdu("800f81ff48000000096d6573736167653137000000096d6573736167653138000000096d65737361676531397000ffdd6102321bc251e4a5190ad5b12b251069d9b4c0843d0f0103ff80ade204")
    navigate_screens(app, group= 4, screen_count= group_counts[4 - 1 ], suffix=suffix)
    app.review.next()
    expected_apdu = "a2ef5aec1ad8cc9b35dee48e8a47e418108dec7652159f3a4314c29d91f172f4645db9554b5a8a565307d9a9e65260957409efef54835573b8fc43d6162f99b8e17a557f7f82c46a53ca7c2be7aa540239d394cd5e9dbf14312c8e1e331a2b099000"
    app.review_confirm_signing(expected_apdu)



def blindsign_review_sign(app: TezosAppScreen):
    app.assert_screen("blindsign_warning_ledger_1")
    app.review.back_to_safety.reject()
    app.assert_screen("summary_review_transaction")
    app.review.next()
    app.assert_screen("tbdm_blind_review_1")
    app.review.next()
    app.assert_screen("tbdm_blind_review_2")
    expected_apdu = "a2ef5aec1ad8cc9b35dee48e8a47e418108dec7652159f3a4314c29d91f172f4645db9554b5a8a565307d9a9e65260957409efef54835573b8fc43d6162f99b8e17a557f7f82c46a53ca7c2be7aa540239d394cd5e9dbf14312c8e1e331a2b099000"
    app.review_confirm_signing(expected_apdu, True)

if __name__ == "__main__":
    app = tezos_app(__file__)

    app.assert_home()
    app.set_expert_mode(initial_status=False)

# Blindsign status OFF
    app.assert_home()
    app.set_blindsigning_status(BlindsigningStatus.OFF)

    if(app.firmware == Firmware.STAX):
        navigate_common(app, skip=False, group_counts=[3, 1, 6, 2])
    else:
        navigate_common(app, skip=False, group_counts=[4, 2, 5, 3])

# Blindsign status For Large Tx only
    app.assert_home()
    app.set_blindsigning_status(BlindsigningStatus.Large_Tx_only)

    if(app.firmware == Firmware.STAX):
        navigate_common(app, skip=False, group_counts=[3, 1, 6, 2])
    else:
        navigate_common(app, skip=False, group_counts=[4, 2, 5, 3])

# Blindsign status ON
    app.assert_home()
    app.set_blindsigning_status(BlindsigningStatus.ON)

    app.send_initialize_msg( "800f000011048000002c800006c18000000080000000")
    send_payload(app, "800f01ffeb0300000000000000000000000000000000000000000000000000000000000000006b00ffdd6102321bc251e4a5190ad5b12b251069d9b4c0843d0b0104020320182716513907b6bab33f905396d031931c07e01bddd780780c1a56b9c086da6c00ffdd6102321bc251e4a5190ad5b12b251069d9b480897a0c0107c08db701000278eb8b6ab9a768579cd5146b480789650c83f28effff0d7570646174655f636f6e6669670000000607070005030a6e00ffdd6102321bc251e4a5190ad5b12b251069d9b4c08db7010d0105ff01ee572f02e5be5d097ba17369789582882e8abb87c900ffdd6102321bc2")
    app.review.next()
    app.assert_screen("tbdm_blindsign_on_screen_1")
    app.review.skip()
    app.assert_screen("skip_review")
    app.review.enable_skip.confirm()
    app.expect_apdu_return("9000")
    app.send_apdu("800f01ffeb51e4a5190ad5b12b251069d9b48092f4010e0106000000fa000000086d65737361676530000000086d65737361676531000000086d65737361676532000000086d65737361676533000000086d65737361676534000000086d65737361676535000000086d65737361676536000000086d65737361676537000000086d65737361676538000000086d65737361676539000000096d6573736167653130000000096d6573736167653131000000096d6573736167653132000000096d6573736167653133000000096d6573736167653134000000096d6573736167653135000000096d6573736167653136")
    app.expect_apdu_return("9000")
    app.send_apdu("800f81ff48000000096d6573736167653137000000096d6573736167653138000000096d65737361676531397000ffdd6102321bc251e4a5190ad5b12b251069d9b4c0843d0f0103ff80ade204")
    blindsign_review_sign(app)

# Blindsign status ON but continue clear signing
    app.assert_home()
    app.set_blindsigning_status(BlindsigningStatus.ON)

    if(app.firmware == Firmware.STAX):
        navigate_common(app, skip=True, group_counts=[3, 1, 6, 2])
    else:
        navigate_common(app, skip=True, group_counts=[4, 2, 5, 3])
