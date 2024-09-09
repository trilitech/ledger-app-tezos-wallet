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

from pathlib import Path

from utils.app import Screen, DEFAULT_ACCOUNT
from utils.message import Message

# Operation (0): SR: originate
# Fee: 0.01 XTZ
# Storage limit: 4
# Kind: arith
# Kernel: 396630396632393532643334353238633733336639343631356366633339626335353536313966633535306464346136376261323230386365386538363761613364313361366566393964666265333263363937346161396132313530643231656361323963333334396535396331336239303831663163313162343430616334643334353564656462653465653064653135613861663632306434633836323437643964313332646531626236646132336435666639643864666664613232626139613834
# Parameters: Pair "1" 2
# Whitelist (0): tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa
# Whitelist (1): tz2WmivuMG8MMRKMEmzKRMMxMApxZQWYNS4W
# Whitelist (2): tz3XMQscBFM9vPmpbYMavMmwxRMUWvWGZMQQ

def test_sign_sc_rollup_originate(app):
    test_name = Path(__file__).stem

    app.setup_expert_mode()
    app.setup_blindsign_off()

    sc_rollup_originate_with_missing_white_list = "030000000000000000000000000000000000000000000000000000000000000000c800ffdd6102321bc251e4a5190ad5b12b251069d9b4904e02030400000000c63966303966323935326433343532386337333366393436313563666333396263353535363139666335353064643461363762613232303863653865383637616133643133613665663939646662653332633639373461613961323135306432316563613239633333343965353963313362393038316631633131623434306163346433343535646564626534656530646531356138616636323064346338363234376439643133326465316262366461323364356666396438646666646132326261396138340000000a07070100000001310002"

    def check_sign(name: str, whitelist: bytes):

        message = Message.from_bytes(sc_rollup_originate_with_missing_white_list + whitelist)

        data = app.sign(DEFAULT_ACCOUNT,
                        message,
                        with_hash=True,
                        path=Path(test_name) / name)

        app.checker.check_signature(
            account=DEFAULT_ACCOUNT,
            message=message,
            with_hash=True,
            data=data)

    # None
    check_sign("no_whitelist", "00")

    # Some []
    check_sign("no_whitelist", "ff00000000")

    # Some [
    #   tz1ixvCiPJYyMjsp2nKBVaq54f6AdbV8hCKa;
    #   tz2WmivuMG8MMRKMEmzKRMMxMApxZQWYNS4W;
    #   tz3XMQscBFM9vPmpbYMavMmwxRMUWvWGZMQQ
    # ]
    check_sign("with_whitelist", "ff0000003f00ffdd6102321bc251e4a5190ad5b12b251069d9b401f6552df4f5ff51c3d13347cab045cfdb8b9bd8030278eb8b6ab9a768579cd5146b480789650c83f28e")

    app.quit()
