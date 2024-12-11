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
from pathlib import Path
import time
from typing import List, Optional, Union

from ragger.firmware import Firmware
from ragger.backend import SpeculosBackend
from ragger.navigator import NavIns, NavInsID, Navigator

from .backend import TezosBackend


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
    def blindsign(cls, backend: TezosBackend) -> "ScreenText":
        """Get blindsign text depending on device."""
        if backend.firmware == Firmware.NANOS:
            return cls.BLINDSIGN_NANOS

        return cls.BLINDSIGN


class TezosNavigator():
    """Class representing Tezos app navigation."""

    _backend: TezosBackend
    _root_dir: Path
    _navigator: Navigator

    def __init__(self,
                 backend: TezosBackend,
                 navigator: Navigator):
        self._backend = backend
        self._root_dir = Path(__file__).resolve().parent.parent
        self._navigator = navigator

    def navigate(self,
                 snap_path: Optional[Path] = None,
                 screen_change_before_first_instruction: bool = False,
                 **kwargs) -> None:
        """Wrapper of `navigator.navigate_and_compare`"""
        self._navigator.navigate_and_compare(
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
        self._navigator.navigate_until_text_and_compare(
            path=self._root_dir,
            test_case_name=snap_path,
            screen_change_before_first_instruction=screen_change_before_first_instruction,
            validation_instructions=validation_instructions,
            **kwargs
        )

    def navigate_while_text_and_compare(
            self,
            navigate_instruction: Union[NavIns, NavInsID],
            text: str,
            validation_instructions: List[Union[NavIns, NavInsID]] = [],
            snap_path: Optional[Path] = None,
            timeout: int = 300,
            screen_change_before_first_instruction: bool = False,
            screen_change_after_last_instruction: bool = True) -> None:
        """Navigate while some text is found on the screen content displayed.

        Function based on
        `ragger.navigator.navigate_until_text_and_compare`

        """
        path = self._root_dir
        test_case_name = snap_path

        idx = 0
        start = time.time()
        if not isinstance(self._backend, SpeculosBackend):
            if timeout == 30:
                timeout = 200

        self._backend.pause_ticker()

        # Wait for screen to change if needed
        self._navigator._run_instruction(
            NavIns(NavInsID.WAIT, (0, )),
            timeout,
            wait_for_screen_change=screen_change_before_first_instruction
        )

        while self._backend.compare_screen_with_text(text):
            remaining = timeout - (time.time() - start)
            if remaining < 0:
                raise TimeoutError(f"Timeout waiting for text {text}")

            # Compare screen, screen already waited
            self._navigator._run_instruction(
                NavIns(NavInsID.WAIT, (0, )),
                remaining,
                wait_for_screen_change=False,
                path=path,
                test_case_name=test_case_name,
                snap_idx=idx
            )

            remaining = timeout - (time.time() - start)
            if remaining < 0:
                raise TimeoutError(f"Timeout waiting for text {text}")

            # Go to the next screen.
            # Don't compare because the text may not be on the screen.
            self._navigator._run_instruction(
                navigate_instruction,
                remaining,
                wait_for_screen_change=True
            )
            idx += 1

        if validation_instructions:
            remaining = timeout - (time.time() - start)
            self._navigator.navigate_and_compare(
                path,
                test_case_name,
                validation_instructions,
                timeout=remaining,
                screen_change_before_first_instruction=True,
                screen_change_after_last_instruction=screen_change_after_last_instruction,
                snap_start_idx=idx
            )

        self._backend.resume_ticker()

    def unsafe_navigate(
            self,
            instructions: List[Union[NavIns, NavInsID]],
            snap_path: Optional[Path] = None,
            timeout: float = 10.0,
            screen_change_before_first_instruction: bool = False,
            screen_change_after_last_instruction: bool = True,
            snap_start_idx: int = 0) -> None:
        """Navigate using instructions but do not wait for screens to
        change.  Only use this function if consecutive screens are the
        same.

        Function based on `ragger.navigator.navigate_and_compare`

        """
        self._backend.pause_ticker()
        self._navigator._run_instruction(
            NavIns(NavInsID.WAIT, (0, )),
            timeout,
            wait_for_screen_change=screen_change_before_first_instruction,
            path=self._root_dir,
            test_case_name=snap_path,
            snap_idx=snap_start_idx
        )
        for idx, instruction in enumerate(instructions):
            if idx + 1 != len(instructions) or screen_change_after_last_instruction:
                self._navigator._run_instruction(
                    instruction,
                    timeout,
                    wait_for_screen_change=False,
                    path=self._root_dir,
                    test_case_name=snap_path,
                    snap_idx=snap_start_idx + idx + 1
                )
            else:
                self._navigator._run_instruction(
                    instruction,
                    timeout,
                    wait_for_screen_change=False,
                    snap_idx=snap_start_idx + idx + 1
                )
        self._backend.resume_ticker()

    def navigate_to_settings(self, **kwargs) -> int:
        """Navigate from Home screen to settings."""
        instructions: List[Union[NavIns, NavInsID]] = [
            # Home
            NavInsID.RIGHT_CLICK,  # Version
            NavInsID.RIGHT_CLICK,  # Settings
            NavInsID.BOTH_CLICK,
        ]
        self.navigate(instructions=instructions, **kwargs)
        snap_start_idx = kwargs['snap_start_idx'] if 'snap_start_idx' in kwargs else 0
        return snap_start_idx + len(instructions)

    def toggle_expert_mode(self, **kwargs) -> int:
        """Enable expert-mode from home screen."""
        go_to_settings_kwargs = kwargs.copy()
        go_to_settings_kwargs['screen_change_after_last_instruction'] = True
        snap_idx = self.navigate_to_settings(**go_to_settings_kwargs)

        instructions: List[Union[NavIns, NavInsID]] = [
            # Expert Mode
            NavInsID.BOTH_CLICK,
            NavInsID.RIGHT_CLICK,  # Blind Sign
            NavInsID.RIGHT_CLICK,  # Back
            NavInsID.BOTH_CLICK,  # Home
        ]
        kwargs['snap_start_idx'] = snap_idx
        kwargs['screen_change_before_first_instruction'] = False
        self.navigate(instructions=instructions, **kwargs)

        return snap_idx + len(instructions)

    def toggle_blindsign(self, **kwargs) -> int:
        """Enable blindsign from home screen."""
        go_to_settings_kwargs = kwargs.copy()
        go_to_settings_kwargs['screen_change_after_last_instruction'] = True
        snap_idx = self.navigate_to_settings(**go_to_settings_kwargs)

        instructions: List[Union[NavIns, NavInsID]] = [
            # Expert Mode
            NavInsID.RIGHT_CLICK,  # Blind Sign
            NavInsID.BOTH_CLICK,
            NavInsID.RIGHT_CLICK,  # Back
            NavInsID.BOTH_CLICK,  # Home
        ]
        kwargs['snap_start_idx'] = snap_idx
        kwargs['screen_change_before_first_instruction'] = False
        self.navigate(instructions=instructions, **kwargs)

        return snap_idx + len(instructions)

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
