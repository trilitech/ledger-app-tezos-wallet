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

import os
import time

from ragger.backend import SpeculosBackend
from ragger.firmware import Firmware
from ragger.firmware.stax.screen import MetaScreen
from ragger.firmware.stax.use_cases import UseCaseHome, UseCaseSettings, UseCaseAddressConfirmation

MAX_ATTEMPTS = 100

SCREEN_HOME_DEFAULT = "home"
SCREEN_INFO_PAGE = "info"

def with_retry(f, attempts=MAX_ATTEMPTS):
    while True:
        try:
            return f()
        except Exception as e:
            if attempts <= 0:
                print("- with_retry: attempts exhausted -")
                raise e
        attempts -= 1
        # Give plenty of time for speculos to update - can take a long time on CI machines
        time.sleep(0.5)

class TezosAppScreen(metaclass=MetaScreen):
    use_case_welcome = UseCaseHome
    use_case_info = UseCaseSettings
    use_case_provide_pk = UseCaseAddressConfirmation

    __backend = None
    __golden = False
    __stax = None
    __snapshotted = []

    def __init__(self, backend, firmware):
        self.__backend = backend
        realpath = os.path.realpath(__file__)
        self.__stax = os.path.dirname(realpath)

    def send_apdu(self, data):
        """Send hex-encoded bytes to the apdu"""
        self.__backend.send_raw(bytes.fromhex(data))

    def expect_apdu_return(self, expected):
        """Expect hex-encoded response from the apdu"""
        response = self.__backend.receive().raw
        expected = bytes.fromhex(expected)
        assert response == expected, f"Expected {expected}, received {response}"

    def assert_screen(self, screen):
        golden = self.__golden and screen not in self.__snapshotted
        if golden:
            self.__snapshotted = self.__snapshotted + [screen]
            input(f"Press ENTER to snapshot {screen}")

        path = f'{self.__stax}/snapshots/{screen}.png'
        def check():
            print(f"- Expecting {screen} -")
            assert self.__backend.compare_screen_with_snapshot(path, golden_run = golden)

        with_retry(check)

    def make_golden(self):
        self.__golden = True

def stax_app() -> TezosAppScreen:
    port = os.environ["PORT"]
    golden = os.getenv("GOLDEN") != None
    backend = SpeculosBackend("__unused__", Firmware.STAX, port = port)
    app = TezosAppScreen(backend, Firmware.STAX)

    if golden:
        app.make_golden()

    return app
