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
from typing import Generator, List
from contextlib import contextmanager
from requests.exceptions import ChunkedEncodingError

from ragger.backend import BackendInterface, SpeculosBackend
from ragger.backend.interface import RaisePolicy
from ragger.firmware import Firmware
from ragger.firmware.touch.element import Center
from ragger.firmware.touch.screen import MetaScreen
from ragger.firmware.touch.use_cases import (
    UseCaseHomeExt,
    UseCaseSettings as OriginalUseCaseSettings,
    UseCaseAddressConfirmation as OriginalUseCaseAddressConfirmation,
    UseCaseReview as OriginalUseCaseReview,
    UseCaseChoice as OriginalUseCaseChoice
)
from ragger.firmware.touch.layouts import ChoiceList
from ragger.firmware.touch.positions import (
    Position,
    STAX_BUTTON_LOWER_LEFT,
    STAX_BUTTON_ABOVE_LOWER_MIDDLE,
    STAX_BUTTON_LOWER_RIGHT,
    STAX_BUTTON_LOWER_MIDDLE,
    FLEX_BUTTON_LOWER_LEFT,
    FLEX_BUTTON_ABOVE_LOWER_MIDDLE
)

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

class UseCaseChoice(OriginalUseCaseChoice):
    """Extension of UseCaseChoice for our app."""

    # Fixed in Ragger v1.21.0
    def reject(self) -> None:
        """Tap on reject button."""
        if self.firmware == Firmware.STAX:
            self.client.finger_touch(*STAX_BUTTON_LOWER_LEFT)
        if self.firmware == Firmware.FLEX:
            super().reject()

class UseCaseReview(OriginalUseCaseReview):
    """Extension of UseCaseReview for our app."""

    reject_tx:        UseCaseChoice
    enable_expert:    UseCaseChoice
    enable_blindsign: UseCaseChoice

    _center: Center

    def __init__(self, client: BackendInterface, firmware: Firmware):
        super().__init__(client, firmware)
        self.reject_tx        = UseCaseChoice(client, firmware)
        self.enable_expert    = UseCaseChoice(client, firmware)
        self.enable_blindsign = UseCaseChoice(client, firmware)
        self._center = Center(client, firmware)

    def next(self) -> None:
        """Pass to the next screen."""
        self._center.swipe_left()

    # Fixed in Ragger v1.21.0
    def tap(self) -> None:
        """Tap on screen."""
        if self.firmware == Firmware.STAX:
            self.client.finger_touch(*STAX_BUTTON_LOWER_RIGHT)
        if self.firmware == Firmware.FLEX:
            super().tap()

    # Fixed in Ragger v1.21.0
    def previous(self) -> None:
        """Tap on screen."""
        if self.firmware == Firmware.STAX:
            self.client.finger_touch(*STAX_BUTTON_LOWER_MIDDLE)
        if self.firmware == Firmware.FLEX:
            super().previous()

    # Fixed in Ragger v1.21.0
    def reject(self) -> None:
        """Tap on reject button."""
        if self.firmware == Firmware.STAX:
            self.client.finger_touch(*STAX_BUTTON_LOWER_LEFT)
        if self.firmware == Firmware.FLEX:
            super().reject()

class UseCaseAddressConfirmation(OriginalUseCaseAddressConfirmation):
    """Extension of UseCaseAddressConfirmation for our app."""

    _center: Center

    QR_POSITIONS = {
        Firmware.STAX: Position(STAX_BUTTON_LOWER_LEFT.x, STAX_BUTTON_ABOVE_LOWER_MIDDLE.y),
        Firmware.FLEX: Position(FLEX_BUTTON_LOWER_LEFT.x, FLEX_BUTTON_ABOVE_LOWER_MIDDLE.y)
    }

    def __init__(self, client: BackendInterface, firmware: Firmware):
        super().__init__(client, firmware)
        self._center = Center(client, firmware)

    def next(self) -> None:
        """Pass to the next screen."""
        self._center.swipe_left()

    @property
    def qr_position(self) -> Position:
        """Position of the qr code."""
        return UseCaseAddressConfirmation.QR_POSITIONS[self.firmware]

    def show_qr(self) -> None:
        """Tap to show qr code."""
        self.client.finger_touch(*self.qr_position)

    # Fixed in Ragger v1.21.0
    def cancel(self) -> None:
        """Tap on cancel button."""
        if self.firmware == Firmware.STAX:
            self.client.finger_touch(*STAX_BUTTON_LOWER_LEFT)
        if self.firmware == Firmware.FLEX:
            super().cancel()

class UseCaseSettings(OriginalUseCaseSettings):
    """Extension of UseCaseSettings for our app."""

    _toggle_list: ChoiceList

    def __init__(self, client: BackendInterface, firmware: Firmware):
        super().__init__(client, firmware)
        self._toggle_list = ChoiceList(client, firmware)

    def toggle_expert_mode(self):
        """Toggle the expert_mode switch."""
        self._toggle_list.choose(1)

    def toggle_blindsigning(self):
        """Toggle the blindsigning switch."""
        self._toggle_list.choose(2)

    def exit(self) -> None:
        """Exits settings."""
        self.multi_page_exit()

    # Fixed in Ragger v1.21.0
    STAX_BUTTON_LOWER_MIDDLE_RIGHT = Position(266, 615)
    def previous(self) -> None:
        """Tap on cancel button."""
        if self.firmware == Firmware.STAX:
            self.client.finger_touch(*UseCaseSettings.STAX_BUTTON_LOWER_MIDDLE_RIGHT)
        if self.firmware == Firmware.FLEX:
            super().previous()

class TezosAppScreen(metaclass=MetaScreen):
    use_case_welcome    = UseCaseHomeExt
    use_case_settings   = UseCaseSettings
    use_case_provide_pk = UseCaseAddressConfirmation
    use_case_review     = UseCaseReview

    welcome:    UseCaseHomeExt
    settings:   UseCaseSettings
    provide_pk: UseCaseAddressConfirmation
    review:     UseCaseReview

    firmware: Firmware
    commit:   str
    version:  str

    __backend:                 BackendInterface
    __golden:                  bool
    __snapshots_path:          str
    __prefixed_snapshots_path: str
    __snapshotted:             List[str]  = []

    def __init__(self,
                 backend: BackendInterface,
                 firmware: Firmware,
                 commit: str,
                 version: str,
                 snapshot_prefix: str,
                 golden: bool = False):
        self.__backend = backend
        realpath = os.path.realpath(__file__)
        self.__snapshots_path = \
            f"{os.path.dirname(realpath)}/snapshots/{firmware.name}"
        self.__prefixed_snapshots_path = \
            f"{self.__snapshots_path}/{Path(snapshot_prefix).stem}"
        self.firmware = firmware
        self.commit = commit
        self.version = version
        self.__golden = golden
        if golden:
            # Setup for golden
            path = f"{self.__prefixed_snapshots_path}"
            Path(path).mkdir(parents=True, exist_ok=True)
            for filename in os.listdir(path):
                os.remove(os.path.join(path, filename))
            path = f"{self.__snapshots_path}"
            home_path=os.path.join(path, "home.png")
            info_path=os.path.join(path, "info.png")
            if os.path.exists(home_path): os.remove(home_path)
            if os.path.exists(info_path): os.remove(info_path)

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
            path = f'{self.__prefixed_snapshots_path}/{screen}.png'
        def check():
            print(f"- Expecting {screen} -")
            assert self.__backend.compare_screen_with_snapshot(path, golden_run = golden)

        with_retry(check)

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
            input("PRESS ENTER to continue next test\n- You may need to reset to home")

    @contextmanager
    def manual_ticker(self) -> Generator[None, None, None]:
        self.__backend.pause_ticker()
        yield
        self.__backend.resume_ticker()

    @contextmanager
    def fading_screen(self, screen) -> Generator[None, None, None]:
        with self.manual_ticker():
            yield
            self.assert_screen(screen)
            self.review.tap() # Not waiting for the screen to fade on its own

    def start_loading_operation(self, first_packet):
        """
        Send the first packet for signing an operation.

        We ensure that the loading operation screen is shown.
        """
        with self.fading_screen("loading_operation"):
            self.send_apdu(first_packet)
        self.expect_apdu_return("9000")

    def review_confirm_signing(self, expected_apdu):
        with self.fading_screen("signing_successful"):
            self.review.confirm()
        self.expect_apdu_return(expected_apdu)

    def enable_expert_mode(self):
        self.assert_screen("enable_expert_mode")
        with self.fading_screen("enabled_expert_mode"):
            self.review.enable_expert.confirm()

    def expert_mode_splash(self):
        self.enable_expert_mode()
        self.assert_screen("expert_mode_splash")

    def review_reject_signing(self, cancel_rejection = False):
        self.review.reject()
        # Rejection confirmation page
        self.assert_screen("confirm_rejection")
        if cancel_rejection:
            self.review.reject_tx.reject()
        else:
            with self.fading_screen("reject_review"):
                self.review.reject_tx.confirm()

    def process_blindsign_warnings(self, landing_screen: str):
        self.assert_screen("unsafe_operation_warning_1")
        self.review.reject()
        self.assert_screen("unsafe_operation_warning_2")
        with self.fading_screen(landing_screen):
            self.review.enable_blindsign.confirm()



def tezos_app(prefix) -> TezosAppScreen:
    port = os.environ["PORT"]
    commit = os.environ["COMMIT_BYTES"]
    version = os.environ["VERSION_BYTES"]
    golden = os.getenv("GOLDEN") != None
    target = os.getenv("TARGET")
    firmware = Firmware.STAX if target == "stax" else Firmware.FLEX
    backend = SpeculosBackend("__unused__", firmware, args=["--api-port", port])
    return TezosAppScreen(backend, firmware, commit, version, prefix, golden)

def assert_home_with_code(app, code):
    app.assert_home()
    app.expect_apdu_failure(code)

def send_initialize_msg(app, apdu):
    app.send_apdu(apdu)
    app.expect_apdu_return("9000")
    app.assert_screen("review_request_sign_operation")

def send_payload(app, apdu):
    app.send_apdu(apdu)
    app.assert_screen("review_request_sign_operation")

def verify_err_reject_response(app, tag):
    verify_reject_response_common(app, tag, "9405")

def verify_reject_response(app, tag):
    verify_reject_response_common(app, tag,"6985")

def verify_reject_response_common(app, tag, err_code):
    app.assert_screen(tag)
    app.review.reject()
    reject_flow(app, err_code)

def reject_flow(app,err_code):
    app.assert_screen("reject_review")
    with app.fading_screen("rejected"):
        app.review.reject_tx.confirm()
    assert_home_with_code(app, err_code)

def index_screen(screen: str, index: int) -> str:
    return screen + "_" + str(index).zfill(2)
