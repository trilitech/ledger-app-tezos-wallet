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
from ragger.backend.interface import RaisePolicy
from ragger.firmware import Firmware
from ragger.firmware.stax.screen import MetaScreen
from ragger.firmware.stax.use_cases import UseCaseHomeExt, UseCaseSettings, UseCaseAddressConfirmation, UseCaseReview
from ragger.firmware.stax.layouts import ChoiceList

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
    use_case_welcome = UseCaseHomeExt
    use_case_info = UseCaseSettings
    use_case_provide_pk = UseCaseAddressConfirmation
    use_case_review = UseCaseReview

    __backend = None
    __golden = False
    __stax = None
    __snapshotted = []
    __settings = None

    def __init__(self, backend, firmware):
        self.__backend = backend
        realpath = os.path.realpath(__file__)
        self.__stax = os.path.dirname(realpath)
        self.__settings = ChoiceList(backend, firmware)

    def send_apdu(self, data):
        """Send hex-encoded bytes to the apdu"""
        self.__backend.send_raw(bytes.fromhex(data))

    def expect_apdu_return(self, expected):
        """Expect hex-encoded response from the apdu"""
        response = self.__backend.receive().raw
        expected = bytes.fromhex(expected)
        assert response == expected, f"Expected {expected}, received {response}"

    def expect_apdu_failure(self, code):
        """Expect failure of 'code'"""
        self.__backend.raise_policy = RaisePolicy.RAISE_NOTHING
        self.expect_apdu_return(code)
        self.__backend.raise_policy = RaisePolicy.RAISE_ALL_BUT_0x9000

    def send_async_apdu(self, data):
        """Send hex-encoded bytes asynchronously to the apdu"""
        return self.__backend.exchange_async_raw(bytes.fromhex(data))

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

    def settings_toggle_blindsigning(self):
        self.__settings.choose(1)

    def start_loading_operation(self, first_packet):
        """
        Send the first packet for signing an operation.

        We ensure that the loading operation screen is shown.
        """
        self.welcome.client.pause_ticker()
        self.send_apdu(first_packet)
        self.assert_screen("loading_operation")
        self.welcome.client.resume_ticker()
        self.expect_apdu_return("9000")

    def review_confirm_signing(self, expected_apdu):
        self.welcome.client.pause_ticker()
        self.review.confirm()
        self.assert_screen("signing_successful")
        self.review.tap()
        self.expect_apdu_return(expected_apdu)

    def review_reject_signing(self):
        self.welcome.client.pause_ticker()
        self.review.reject()
        self.assert_screen("reject_review")
        self.review.tap()
        self.welcome.client.resume_ticker()

def stax_app() -> TezosAppScreen:
    port = os.environ["PORT"]
    golden = os.getenv("GOLDEN") != None
    backend = SpeculosBackend("__unused__", Firmware.STAX, port = port)
    app = TezosAppScreen(backend, Firmware.STAX)

    if golden:
        app.make_golden()

    return app
