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

import os
import time

from pathlib import Path
from contextlib import contextmanager
from requests.exceptions import ChunkedEncodingError

from ragger.backend import SpeculosBackend
from ragger.backend.interface import RaisePolicy
from ragger.firmware import Firmware
from ragger.firmware.touch.screen import MetaScreen
from ragger.firmware.touch.use_cases import UseCaseHomeExt, UseCaseSettings, UseCaseAddressConfirmation, UseCaseReview, UseCaseChoice
from ragger.firmware.touch.layouts import ChoiceList
from ragger.firmware.touch.positions import STAX_BUTTON_LOWER_LEFT, STAX_BUTTON_LOWER_RIGHT, STAX_BUTTON_ABOVE_LOWER_MIDDLE, STAX_BUTTON_LOWER_MIDDLE

MAX_ATTEMPTS = 50

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
    use_case_choice = UseCaseChoice

    __backend = None
    __golden = False
    __stax = None
    __snapshotted = []
    __settings = None

    def __init__(self, backend, firmware, commit, version, snapshot_prefix):
        self.__backend = backend
        realpath = os.path.realpath(__file__)
        self.__snapshots_path = f"{os.path.dirname(realpath)}/snapshots"
        self.__stax = f"{os.path.dirname(realpath)}/snapshots/{Path(snapshot_prefix).stem}"
        self.__settings = ChoiceList(backend, firmware)
        self.commit = commit
        self.version = version

    def send_apdu(self, data):
        """Send hex-encoded bytes to the apdu"""
        self.__backend.send_raw(bytes.fromhex(data))

    def expect_apdu_return(self, expected):
        """Expect hex-encoded response from the apdu"""
        response = self.__backend.receive().raw
        expected = bytes.fromhex(expected)
        assert response == expected, f"Expected {expected.hex()}, received {response.hex()}"

    def expect_apdu_failure(self, code):
        """Expect failure of 'code'"""
        self.__backend.raise_policy = RaisePolicy.RAISE_NOTHING
        self.expect_apdu_return(code)
        self.__backend.raise_policy = RaisePolicy.RAISE_ALL_BUT_0x9000

    def send_async_apdu(self, data):
        """Send hex-encoded bytes asynchronously to the apdu"""
        return self.__backend.exchange_async_raw(bytes.fromhex(data))

    def assert_screen(self, screen, fixed: bool = False):
        golden = self.__golden and screen not in self.__snapshotted
        if golden:
            self.__snapshotted = self.__snapshotted + [screen]
            input(f"Press ENTER to snapshot {screen}")

        if fixed:
            path = f'{self.__snapshots_path}/{screen}.png'
        else:
            path = f'{self.__stax}/{screen}.png'
        def check():
            print(f"- Expecting {screen} -")
            assert self.__backend.compare_screen_with_snapshot(path, golden_run = golden)

        with_retry(check)

    def make_golden(self):
        self.__golden = True
        path = f"{self.__stax}/"
        Path(path).mkdir(parents=True, exist_ok=True)
        for filename in os.listdir(path):
            os.remove(os.path.join(path, filename))
        path = f"{self.__snapshots_path}"
        home_path=os.path.join(path, "home.png")
        info_path=os.path.join(path, "info.png")
        if os.path.exists(home_path): os.remove(home_path)
        if os.path.exists(info_path): os.remove(info_path)

    def assert_home(self):
        self.assert_screen("home", True)

    def assert_info(self):
        self.assert_screen("info", True)

    def assert_settings(self,
                        blindsigning = False,
                        expert_mode = False):
        suffix=""
        if blindsigning:
            suffix += "_blindsigning"
        if expert_mode:
            suffix += "_expert"
        if suffix != "":
            suffix += "_on"
        self.assert_screen("settings" + suffix)

    def quit(self):
        if os.getenv("NOQUIT") == None:
            try:
                self.welcome.quit()
            except ConnectionError:
                pass
            except ChunkedEncodingError:
                pass

        else:
            input(f"PRESS ENTER to continue next test\n- You may need to reset to home")

    def settings_toggle_expert_mode(self):
        self.__settings.choose(1)

    def settings_toggle_blindsigning(self):
        self.__settings.choose(2)

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

    def enable_expert_mode(self, expert_enabled=False):
        if not expert_enabled:
            self.assert_screen("enable_expert_mode")
            self.welcome.client.finger_touch(STAX_BUTTON_ABOVE_LOWER_MIDDLE.x, STAX_BUTTON_ABOVE_LOWER_MIDDLE.y)
            self.assert_screen("enabled_expert_mode")

    def expert_mode_splash(self, expert_enabled=False):
        self.enable_expert_mode(expert_enabled)
        self.review.tap()
        self.assert_screen("expert_mode_splash")



    def review_reject_signing(self, confirmRejection = True):
        self.welcome.client.finger_touch(STAX_BUTTON_LOWER_LEFT.x, STAX_BUTTON_LOWER_RIGHT.y)
        # Rejection confirmation page
        self.assert_screen("confirm_rejection")
        if confirmRejection:
            self.welcome.client.pause_ticker()
            self.welcome.client.finger_touch(STAX_BUTTON_ABOVE_LOWER_MIDDLE.x, STAX_BUTTON_ABOVE_LOWER_MIDDLE.y)
            self.assert_screen("reject_review")
            self.review.tap()
            self.welcome.client.resume_ticker()
        else:
            self.welcome.client.finger_touch(STAX_BUTTON_LOWER_MIDDLE.x, STAX_BUTTON_LOWER_MIDDLE.y)

def stax_app(prefix) -> TezosAppScreen:
    port = os.environ["PORT"]
    commit = os.environ["COMMIT_BYTES"]
    version = os.environ["VERSION_BYTES"]
    golden = os.getenv("GOLDEN") != None
    backend = SpeculosBackend("__unused__", Firmware.STAX,args=["--api-port", port])
    app = TezosAppScreen(backend, Firmware.STAX, commit, version, prefix)

    if golden:
        app.make_golden()

    return app

def assert_home_with_code(app, code):
    app.assert_home()
    app.expect_apdu_failure(code)

def send_initialize_msg(app, apdu):
    app.send_apdu(apdu)
    app.expect_apdu_return("9000")

    app.assert_screen("review_request_sign_operation");

def send_payload(app, apdu):
    app.send_apdu(apdu)
    app.assert_screen("review_request_sign_operation");

def verify_err_reject_response(app, tag):
    verify_reject_response_common(app,tag,"9405")

def verify_reject_response(app, tag):
    verify_reject_response_common(app, tag,"6985")

def verify_reject_response_common(app, tag, err_code):
    app.assert_screen(tag)
    app.review.reject()
    app.assert_screen("reject_review")
    app.welcome.client.pause_ticker()
    app.welcome.client.finger_touch(STAX_BUTTON_ABOVE_LOWER_MIDDLE.x, STAX_BUTTON_ABOVE_LOWER_MIDDLE.y)
    app.assert_screen("rejected")
    app.review.tap()
    app.welcome.client.resume_ticker()
    assert_home_with_code(app, err_code)
