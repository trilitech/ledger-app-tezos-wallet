# Copyright 2023 Trilitech <contact@trili.tech>
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

"""Tezos app backend."""

from enum import Enum
from io import BytesIO
from pathlib import Path
import time
from typing import List, Optional, Union

import requests
from ragger.backend import SpeculosBackend
from ragger.navigator import NavIns, NavInsID, NanoNavigator

from .account import Account, SigType
from .backend import TezosBackend


MAX_ATTEMPTS = 50

def with_retry(f, attempts=MAX_ATTEMPTS):
    """Try f until it succeeds or has been executed attempts times."""
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

class SpeculosTezosBackend(TezosBackend, SpeculosBackend):
    """Class representing Tezos app running on Speculos."""

    # speculos can be slow to start up in a slow environment.
    # Here, we expect a little more
    def __enter__(self) -> "SpeculosTezosBackend":
        try:
            super().__enter__()
            return self
        except Exception:
            process = self._client.process
            try:
                with_retry(self._client._wait_until_ready, attempts=5)
                super().__enter__()
            except Exception as e:
                self._client.stop()
                # do not forget to close the first process
                self._client.process = process
                self._client.stop()
                raise e
            # replace the new process by the first one
            self._client.process.kill()
            self._client.process.wait()
            self._client.process = process
            return self

class Screen(str, Enum):
    """Class representing common, known app screens."""

    HOME = "home"
    VERSION = "version"
    SETTINGS = "settings"
    SETTINGS_EXPERT_MODE_DISABLED = "settings_expert_mode_disabled"
    SETTINGS_EXPERT_MODE_ENABLED = "settings_expert_mode_enabled"
    SETTINGS_BLINDSIGN_ON = "settings_blindsign_on"
    SETTINGS_BLINDSIGN_OFF = "settings_blindsign_off"
    SETTINGS_BACK = "back"
    QUIT = "quit"

    def __str__(self) -> str:
        return self.value

class ScreenText(str, Enum):
    """Class representing common, known app screen's text."""

    HOME = "Application"
    PUBLIC_KEY_APPROVE = "Approve"
    PUBLIC_KEY_REJECT = "Reject"
    SIGN_ACCEPT = "Accept"
    SIGN_REJECT = "Reject"
    ACCEPT_RISK = "Accept risk"
    BACK_HOME = "Home"
    BLINDSIGN = "blindsign"
    BLINDSIGN_NANOS = "Blindsign"

    @classmethod
    def blindsign(cls, backend: SpeculosTezosBackend) -> "ScreenText":
        """Get blindsign text depending on device."""
        if backend.firmware.device == "nanos":
            return cls.BLINDSIGN_NANOS

        return cls.BLINDSIGN

class TezosAppScreen():
    """Class representing Tezos app management."""

    backend: SpeculosTezosBackend
    _root_dir: Path
    snapshots_dir: Path
    tmp_snapshots_dir: Path
    snapshotted: List[str]
    golden_run: bool
    navigator: NanoNavigator

    def __init__(self,
                 backend: SpeculosTezosBackend,
                 golden_run: bool):
        self.backend = backend
        self._root_dir = Path(__file__).resolve().parent.parent
        self.snapshots_dir = self._root_dir / "snapshots" / backend.firmware.name
        self.tmp_snapshots_dir = self._root_dir / "snapshots-tmp" / backend.firmware.name
        if not self.snapshots_dir.is_dir() and golden_run:
            self.snapshots_dir.mkdir(parents=True)
        if not self.tmp_snapshots_dir.is_dir():
            self.tmp_snapshots_dir.mkdir(parents=True)
        self.snapshotted = []

        self.golden_run = golden_run
        self.navigator = NanoNavigator(backend, backend.firmware, golden_run)

    def __enter__(self) -> "TezosAppScreen":
        with_retry(self.backend.__enter__, attempts=3)
        return self

    def __exit__(self, *args):
        self.backend.__exit__(*args)

    def assert_screen(self,
                      screen: Union[str, Screen],
                      path: Optional[Union[str, Path]] = None) -> None:
        """Check if the screen is the one expected."""
        golden_run = self.golden_run and screen not in self.snapshotted
        if golden_run:
            self.snapshotted = self.snapshotted + [screen]
            input(f"Press ENTER to snapshot {screen}")

        snapshots_dir = self.snapshots_dir if path is None \
            else self.snapshots_dir / path
        tmp_snapshots_dir = self.tmp_snapshots_dir if path is None \
            else self.tmp_snapshots_dir / path

        if not snapshots_dir.is_dir() and golden_run:
            snapshots_dir.mkdir(parents=True)
        if not tmp_snapshots_dir.is_dir():
            tmp_snapshots_dir.mkdir(parents=True)

        path = snapshots_dir / f'{screen}.png'
        tmp_path = tmp_snapshots_dir / f'{screen}.png'
        def check():
            print(f"- Expecting {screen} -")
            assert self.backend.compare_screen_with_snapshot(
                path,
                tmp_snap_path=tmp_path,
                golden_run=golden_run)

        with_retry(check)
        self.backend._last_screenshot = BytesIO(self.backend._client.get_screenshot())

    def setup_expert_mode(self) -> None:
        """Enable expert-mode from home screen."""
        self.assert_screen(Screen.HOME)
        self.backend.right_click()
        self.assert_screen(Screen.VERSION)
        self.backend.right_click()
        self.assert_screen(Screen.SETTINGS)
        self.backend.both_click()
        self.assert_screen(Screen.SETTINGS_EXPERT_MODE_DISABLED)
        self.backend.both_click()
        self.assert_screen(Screen.SETTINGS_EXPERT_MODE_ENABLED)
        self.backend.left_click()
        self.assert_screen(Screen.SETTINGS_BACK)
        self.backend.both_click()
        self.assert_screen(Screen.HOME)

    def setup_blindsign_on(self) -> None:
        """Enable blindsign from home screen."""
        self.assert_screen(Screen.HOME)
        self.backend.right_click()
        self.assert_screen(Screen.VERSION)
        self.backend.right_click()
        self.assert_screen(Screen.SETTINGS)
        self.backend.both_click()
        # expert_mode screen
        self.backend.right_click()
        self.assert_screen(Screen.SETTINGS_BLINDSIGN_OFF)
        self.backend.both_click()
        self.assert_screen(Screen.SETTINGS_BLINDSIGN_ON)
        self.backend.right_click()
        self.assert_screen(Screen.SETTINGS_BACK)
        self.backend.both_click()
        self.assert_screen(Screen.HOME)

    def _quit(self) -> None:
        """Ensure quiting exit the app."""
        self.assert_screen(Screen.QUIT)
        try:
            self.backend.both_click()
            assert False, "Must have lost connection with speculos"
        except requests.exceptions.ConnectionError:
            pass

    def quit(self) -> None:
        """Quit the app from home screen."""
        self.assert_screen(Screen.HOME)
        self.backend.right_click()
        self.assert_screen(Screen.VERSION)
        self.backend.right_click()
        self.assert_screen(Screen.SETTINGS)
        self.backend.right_click()
        self._quit()

    def navigate(self,
                 snap_path: Optional[Path] = None,
                 screen_change_before_first_instruction: bool = False,
                 **kwargs) -> None:
        """Wrapper of `navigator.navigate_and_compare`"""
        self.navigator.navigate_and_compare(
            path=self._root_dir,
            test_case_name=snap_path,
            screen_change_before_first_instruction=screen_change_before_first_instruction,
            **kwargs
        )

    def navigate_until_text(self,
                            snap_path: Optional[Path] = None,
                            screen_change_before_first_instruction: bool = False,
                            validation_instructions: List[Union[NavIns, NavInsID]] = [],
                            **kwargs) -> None:
        """Wrapper of `navigator.navigate_until_text_and_compare`"""
        self.navigator.navigate_until_text_and_compare(
            path=self._root_dir,
            test_case_name=snap_path,
            screen_change_before_first_instruction=screen_change_before_first_instruction,
            validation_instructions=validation_instructions,
            **kwargs
        )

    def navigate_forward(self, **kwargs) -> None:
        """Navigate forward until the text is found."""
        self.navigate_until_text(
            navigate_instruction=NavInsID.RIGHT_CLICK,
            **kwargs
        )

    def _navigate_review(
            self,
            screen_change_before_first_instruction=True,
            screen_change_after_last_instruction=False,
            **kwargs
    ) -> None:
        """Helper to navigate forward in a new flow."""
        self.navigate_forward(
            screen_change_before_first_instruction=screen_change_before_first_instruction,
            screen_change_after_last_instruction=screen_change_after_last_instruction,
            **kwargs
        )

    def accept_public_key(self, **kwargs) -> None:
        """Navigate through public key flow and accept public key"""
        self._navigate_review(
            text=ScreenText.PUBLIC_KEY_APPROVE,
            validation_instructions=[NavInsID.BOTH_CLICK],
            **kwargs
        )

    def reject_public_key(self, **kwargs) -> None:
        """Navigate through public key flow and reject"""
        self._navigate_review(
            text=ScreenText.PUBLIC_KEY_REJECT,
            validation_instructions=[NavInsID.BOTH_CLICK],
            **kwargs
        )

    def accept_sign(self, **kwargs) -> None:
        """Navigate through signing flow and accept to sign"""
        self._navigate_review(
            text=ScreenText.SIGN_ACCEPT,
            validation_instructions=[NavInsID.BOTH_CLICK],
            **kwargs
        )

    def reject_sign(self, **kwargs) -> None:
        """Navigate through signing flow and reject."""
        self._navigate_review(
            text=ScreenText.SIGN_REJECT,
            validation_instructions=[NavInsID.BOTH_CLICK],
            **kwargs
        )

    def hard_reject_sign(self, **kwargs) -> None:
        """Navigate through signing flow and until a hard reject send
        back to home."""
        self._navigate_review(
            text=ScreenText.HOME,
            validation_instructions=[],
            **kwargs
        )

    def expert_reject_sign(self, **kwargs) -> None:
        """Navigate through the signing expert requirement flow and reject."""
        self._navigate_review(
            text=ScreenText.BACK_HOME,
            validation_instructions=[NavInsID.BOTH_CLICK],
            **kwargs
        )

    def accept_sign_risk(self, **kwargs) -> None:
        """Navigate through signing risk warning flow and accept risk."""
        self._navigate_review(
            text=ScreenText.ACCEPT_RISK,
            validation_instructions=[NavInsID.BOTH_CLICK],
            **kwargs
        )

DEFAULT_SEED = 'zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra'

DEFAULT_ACCOUNT = Account("m/44'/1729'/0'/0'",
                          SigType.ED25519,
                          "edpkuXX2VdkdXzkN11oLCb8Aurdo1BTAtQiK8ZY9UPj2YMt3AHEpcY")
