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

from utils.apdu import *
from utils.app import *

# Operation (0): Failing noop
# Message: 396630396632393532643334353238633733336639343631356366633339626335353536313966633535306464346136376261323230386365386538363761613364313361366566393964666265333263363937346161396132313530643231656361323963333334396535396331336239303831663163313162343430616334643334353564656462653465653064653135613861663632306434633836323437643964313332646531626236646132336435666639643864666664613232626139613834

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        data = app.sign(DEFAULT_ACCOUNT,
                        "03000000000000000000000000000000000000000000000000000000000000000011000000c6396630396632393532643334353238633733336639343631356366633339626335353536313966633535306464346136376261323230386365386538363761613364313361366566393964666265333263363937346161396132313530643231656361323963333334396535396331336239303831663163313162343430616334643334353564656462653465653064653135613861663632306434633836323437643964313332646531626236646132336435666639643864666664613232626139613834",
                        with_hash=True,
                        path=test_name)

        app.check_signature_with_hash(
            hash="469326f8740bcf4a8c8cbd773117c3fc43997ebaca5ee2f959df380c84303c41",
            signature="e62b6ead4edc86864554ba9f262330da7def7b212ab73e2f53e1a0972b31ea7a2de2b3b3c7179021e7c9bf30c6baaffa0962ee7b7423e2e2c312f79a4bd6d905",
            data=data)

        app.quit()
