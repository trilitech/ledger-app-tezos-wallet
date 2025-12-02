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

from enum import auto
from pathlib import Path
import time
from typing import Any, Callable, Dict, List, Optional, Union

from ledgered.devices import Device, DeviceType
from ragger.backend import BackendInterface, SpeculosBackend
from ragger.firmware.touch.element import Center
from ragger.firmware.touch.layouts import ChoiceList, RightHeader
from ragger.firmware.touch.positions import (
    Position,
    STAX_X_CENTER,
    FLEX_X_CENTER,
    STAX_BUTTON_LOWER_LEFT,
    STAX_BUTTON_ABOVE_LOWER_MIDDLE,
    FLEX_BUTTON_LOWER_LEFT,
    FLEX_BUTTON_ABOVE_LOWER_MIDDLE
)
from ragger.firmware.touch.screen import MetaScreen
from ragger.firmware.touch.use_cases import (
    UseCaseHome,
    UseCaseSettings as OriginalUseCaseSettings,
    UseCaseAddressConfirmation as OriginalUseCaseAddressConfirmation,
    UseCaseReview as OriginalUseCaseReview,
    UseCaseChoice,
    UseCaseViewDetails
)
from ragger.navigator import BaseNavInsID, NavIns, NavInsID, Navigator

from .backend import TezosBackend


class UseCaseSettings(OriginalUseCaseSettings, metaclass=MetaScreen):
    """Custom UseCaseSettings."""

    layout__toggle_list = ChoiceList

    _toggle_list: ChoiceList

    def __init__(self, client: BackendInterface, device: Device):
        # `MetaScreen` require an explicit __init__ function
        super().__init__(client, device)

    def toggle_expert_mode(self):
        """Toggle the expert_mode switch."""
        self._toggle_list.choose(1)

    def toggle_blindsigning(self):
        """Toggle the blindsigning switch."""
        self._toggle_list.choose(2)

    def exit(self) -> None:
        """Exit settings."""
        self.multi_page_exit()


class UseCaseAddressConfirmation(OriginalUseCaseAddressConfirmation):
    """Custom UseCaseAddressConfirmation."""

    QR_POSITIONS = {
        DeviceType.STAX: Position(
            STAX_BUTTON_LOWER_LEFT.x,
            STAX_BUTTON_ABOVE_LOWER_MIDDLE.y
        ),
        DeviceType.FLEX: Position(
            FLEX_BUTTON_LOWER_LEFT.x,
            FLEX_BUTTON_ABOVE_LOWER_MIDDLE.y
        )
    }

    def show_qr(self) -> None:
        """Tap to show qr code."""
        self.client.finger_touch(*self.QR_POSITIONS[self.device.type])


class UseCaseReview(OriginalUseCaseReview, metaclass=MetaScreen):
    """Custom UseCaseReview."""

    use_case_reject_choice = UseCaseChoice
    use_case_enable_expert_choice = UseCaseChoice
    use_case_enable_blindsign_choice = UseCaseChoice
    use_case_skip_choice = UseCaseChoice
    use_case_back_to_safety = UseCaseChoice
    use_case_details = UseCaseViewDetails
    layout__skip_header = RightHeader

    reject_choice:           UseCaseChoice
    enable_expert_choice:    UseCaseChoice
    enable_blindsign_choice: UseCaseChoice
    skip_choice:             UseCaseChoice
    back_to_safety:          UseCaseChoice
    details:                 UseCaseViewDetails
    _skip_header:            RightHeader

    # The ‘more’ button is positioned just below the very long
    # data displayed.  As the size of the data is not fixed, the
    # position of the button cannot be defined statically. The
    # static positions below are defined for the test data.
    MORE_POSITIONS = {
        DeviceType.STAX: Position(STAX_X_CENTER, 390),
        DeviceType.FLEX: Position(FLEX_X_CENTER, 350)
    }

    def __init__(self, client: BackendInterface, device: Device):
        # `MetaScreen` require an explicit __init__ function
        super().__init__(client, device)

    def show_more(self) -> None:
        """Tap to show more."""
        self.client.finger_touch(*self.MORE_POSITIONS[self.device.type])

    def skip(self) -> None:
        """Press the skip button."""
        self._skip_header.tap()


class TezosNavInsID(BaseNavInsID):
    """Custom NavInsID."""

    # UseCaseSettings
    SETTINGS_TOGGLE_EXPERT_MODE = auto()
    SETTINGS_TOGGLE_BLINDSIGNING = auto()
    SETTINGS_EXIT = auto()
    # UseCaseAddressConfirmation
    REVIEW_PK_SHOW_QR = auto()
    # UseCaseReview
    REVIEW_TX_NEXT = auto()
    REVIEW_TX_SHOW_MORE = auto()
    REVIEW_TX_SKIP = auto()
    REJECT_CHOICE_CONFIRM = auto()
    REJECT_CHOICE_RETURN = auto()
    EXPERT_CHOICE_ENABLE = auto()
    EXPERT_CHOICE_REJECT = auto()
    BLINDSIGN_CHOICE_ENABLE = auto()
    BLINDSIGN_CHOICE_REJECT = auto()
    SKIP_CHOICE_CONFIRM = auto()
    SKIP_CHOICE_REJECT = auto()
    WARNING_CHOICE_SAFETY = auto()
    WARNING_CHOICE_BLINDSIGN = auto()


class TezosNavigator(metaclass=MetaScreen):
    """Class representing Tezos app navigation."""

    use_case_home = UseCaseHome
    use_case_settings = UseCaseSettings
    use_case_review_pk = UseCaseAddressConfirmation
    use_case_review_tx = UseCaseReview
    element_center = Center

    home:      UseCaseHome
    settings:  UseCaseSettings
    review_pk: UseCaseAddressConfirmation
    review_tx: UseCaseReview
    center:    Center

    _backend: TezosBackend
    _device: Device
    _navigator: Navigator
    _root_dir: Path

    def __init__(self,
                 backend: TezosBackend,
                 device: Device,
                 navigator: Navigator):
        self._backend = backend
        self._device = device
        self._navigator = navigator

        if self._device.is_nano:
            self._navigator.add_callback(TezosNavInsID.REVIEW_TX_NEXT, self._backend.right_click)
        else:
            tezos_callbacks: Dict[BaseNavInsID, Callable[..., Any]] = {
                TezosNavInsID.SETTINGS_TOGGLE_EXPERT_MODE: self.settings.toggle_expert_mode,
                TezosNavInsID.SETTINGS_TOGGLE_BLINDSIGNING: self.settings.toggle_blindsigning,
                TezosNavInsID.SETTINGS_EXIT: self.settings.exit,
                TezosNavInsID.REVIEW_PK_SHOW_QR: self.review_pk.show_qr,
                TezosNavInsID.REVIEW_TX_NEXT: self._ignore_processing(self.center.swipe_left),
                TezosNavInsID.REVIEW_TX_SHOW_MORE: self.review_tx.show_more,
                TezosNavInsID.REVIEW_TX_SKIP: self.review_tx.skip,
                TezosNavInsID.REJECT_CHOICE_CONFIRM: self.review_tx.reject_choice.confirm,
                TezosNavInsID.REJECT_CHOICE_RETURN: self.review_tx.reject_choice.reject,
                TezosNavInsID.EXPERT_CHOICE_ENABLE: self.review_tx.enable_expert_choice.confirm,
                TezosNavInsID.EXPERT_CHOICE_REJECT: self.review_tx.enable_expert_choice.reject,
                TezosNavInsID.BLINDSIGN_CHOICE_ENABLE: self.review_tx.enable_blindsign_choice.confirm,
                TezosNavInsID.BLINDSIGN_CHOICE_REJECT: self.review_tx.enable_blindsign_choice.reject,
                TezosNavInsID.SKIP_CHOICE_CONFIRM: self._ignore_processing(self.review_tx.skip_choice.confirm),
                TezosNavInsID.SKIP_CHOICE_REJECT: self.review_tx.skip_choice.reject,
                TezosNavInsID.WARNING_CHOICE_SAFETY: self.review_tx.back_to_safety.confirm,
                TezosNavInsID.WARNING_CHOICE_BLINDSIGN: self.review_tx.back_to_safety.reject,
            }
            self._navigator._callbacks.update(tezos_callbacks)
        self._root_dir = Path(__file__).resolve().parent.parent

    def _ignore_processing(self, callback: Callable):
        """Wrapper to ignore the `Proccessing` screen"""
        def wrapper(*args, **kwargs):
            if not isinstance(self._backend, SpeculosBackend):
                callback(*args, **kwargs)
            else:
                last_screenshot = self._backend._last_screenshot
                callback(*args, **kwargs)
                self._backend.wait_for_screen_change()
                if self._backend.compare_screen_with_text("^(Processing|Loading operation)$"):
                    self._backend.send_tick()
                    # Wait a text that is not "Processing"
                    self._backend.wait_for_text_on_screen("^(?!Processing$|Loading operation$).*")
                self._backend._last_screenshot = last_screenshot
        return wrapper

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
                            validation_instructions: List[Union[NavIns, BaseNavInsID]] = [],
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
            navigate_instruction: Union[NavIns, BaseNavInsID],
            text: str,
            validation_instructions: List[Union[NavIns, BaseNavInsID]] = [],
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
            instructions: List[Union[NavIns, BaseNavInsID]],
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
        instructions: List[Union[NavIns, BaseNavInsID]] = []
        if self._device.is_nano:
            instructions = [
                # Home
                NavInsID.RIGHT_CLICK,  # Version
                NavInsID.RIGHT_CLICK,  # Settings
                NavInsID.BOTH_CLICK,
            ]
        else:
            instructions = [NavInsID.USE_CASE_HOME_SETTINGS]
        self.navigate(instructions=instructions, **kwargs)
        snap_start_idx = kwargs['snap_start_idx'] if 'snap_start_idx' in kwargs else 0
        return snap_start_idx + len(instructions)

    def toggle_expert_mode(self, **kwargs) -> int:
        """Enable expert-mode from home screen."""
        go_to_settings_kwargs = kwargs.copy()
        go_to_settings_kwargs['screen_change_after_last_instruction'] = True
        snap_idx = self.navigate_to_settings(**go_to_settings_kwargs)

        instructions: List[Union[NavIns, BaseNavInsID]] = []
        if self._device.is_nano:
            instructions = [
                # Expert Mode
                NavInsID.BOTH_CLICK,
                NavInsID.RIGHT_CLICK,  # Blind Sign
                NavInsID.RIGHT_CLICK,  # Back
                NavInsID.BOTH_CLICK,  # Home
            ]
        else:
            instructions = [
                TezosNavInsID.SETTINGS_TOGGLE_EXPERT_MODE,
                TezosNavInsID.SETTINGS_EXIT,
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

        instructions: List[Union[NavIns, BaseNavInsID]] = []
        if self._device.is_nano:
            instructions = [
                # Expert Mode
                NavInsID.RIGHT_CLICK,  # Blind Sign
                NavInsID.BOTH_CLICK,
                NavInsID.RIGHT_CLICK,  # Back
                NavInsID.BOTH_CLICK,  # Home
            ]
        else:
            instructions = [
                TezosNavInsID.SETTINGS_TOGGLE_BLINDSIGNING,
                TezosNavInsID.SETTINGS_EXIT,
            ]
        kwargs['snap_start_idx'] = snap_idx
        kwargs['screen_change_before_first_instruction'] = False
        self.navigate(instructions=instructions, **kwargs)

        return snap_idx + len(instructions)

    def navigate_forward(self, **kwargs) -> None:
        """Navigate forward until the text is found."""
        self.navigate_until_text(
            navigate_instruction=TezosNavInsID.REVIEW_TX_NEXT,
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

    def accept_public_key(self, show_qr: bool = False, **kwargs) -> None:
        """Navigate through public key flow and accept public key"""
        validation_instructions: List[Union[NavIns, BaseNavInsID]] = []
        if self._device.is_nano:
            text = "^Approve$"
            validation_instructions = [NavInsID.BOTH_CLICK]
        else:
            text = "^Confirm$"
            if show_qr:
                validation_instructions += [
                    TezosNavInsID.REVIEW_PK_SHOW_QR,
                    NavInsID.USE_CASE_ADDRESS_CONFIRMATION_EXIT_QR,
                ]
            validation_instructions += [
                NavInsID.USE_CASE_ADDRESS_CONFIRMATION_CONFIRM,
                NavInsID.USE_CASE_STATUS_DISMISS,
            ]
        self._navigate_review(
            text=text,
            validation_instructions=validation_instructions,
            **kwargs
        )

    def reject_public_key(self, **kwargs) -> None:
        """Navigate through public key flow and reject"""
        validation_instructions: List[Union[NavIns, BaseNavInsID]] = []
        if self._device.is_nano:
            text = "^Reject$"
            validation_instructions = [NavInsID.BOTH_CLICK]
        else:
            text = "^Confirm$"
            validation_instructions = [
                NavInsID.USE_CASE_ADDRESS_CONFIRMATION_CANCEL,
                NavInsID.USE_CASE_STATUS_DISMISS,
            ]
        self._navigate_review(
            text=text,
            validation_instructions=validation_instructions,
            **kwargs
        )

    def accept_sign(self, **kwargs) -> None:
        """Navigate through signing flow and accept to sign"""
        validation_instructions: List[Union[NavIns, BaseNavInsID]] = []
        if self._device.is_nano:
            text = "^Accept$"
            if self._device.type == DeviceType.NANOS:
                text = "^Accept and send$"
            validation_instructions = [NavInsID.BOTH_CLICK]
        else:
            text = "^Hold to sign$"
            validation_instructions = [
                NavInsID.USE_CASE_REVIEW_CONFIRM,
                NavInsID.USE_CASE_STATUS_DISMISS,
            ]
        self._navigate_review(
            text=text,
            validation_instructions=validation_instructions,
            **kwargs
        )

    def reject_sign(self, **kwargs) -> None:
        """Navigate through signing flow and reject."""
        validation_instructions: List[Union[NavIns, BaseNavInsID]] = []
        if self._device.is_nano:
            text = "^Reject$"
            validation_instructions = [NavInsID.BOTH_CLICK]
        else:
            text = "^Hold to sign$"
            validation_instructions = [
                NavInsID.USE_CASE_REVIEW_REJECT,
                TezosNavInsID.REJECT_CHOICE_CONFIRM,
                NavInsID.USE_CASE_STATUS_DISMISS,
            ]
        self._navigate_review(
            text=text,
            validation_instructions=validation_instructions,
            **kwargs
        )

    def hard_reject_sign(self, **kwargs) -> None:
        """Navigate through signing flow and until a hard reject send
        back to home."""
        if self._device.is_nano:
            text = "^Application$"
        else:
            text = "^Tezos Wallet$"
        self._navigate_review(
            text=text,
            validation_instructions=[],
            **kwargs
        )

    def expert_accept_sign(self, **kwargs) -> None:
        """Navigate through the signing expert requirement flow and accept.
        Only available for Touch devices, fail otherwise.
        """
        assert not self._device.is_nano, "Skip available only on Touch devices"
        self._navigate_review(
            text="^Enable expert mode$",
            validation_instructions=[
                TezosNavInsID.EXPERT_CHOICE_ENABLE,
                NavInsID.USE_CASE_STATUS_DISMISS,
            ],
            **kwargs
        )

    def expert_reject_sign(self, **kwargs) -> None:
        """Navigate through the signing expert requirement flow and reject."""
        validation_instructions: List[Union[NavIns, BaseNavInsID]] = []
        if self._device.is_nano:
            text = "^Home$"
            validation_instructions = [NavInsID.BOTH_CLICK]
        else:
            text = "^Enable expert mode$"
            validation_instructions = [
                TezosNavInsID.EXPERT_CHOICE_REJECT,
                TezosNavInsID.REJECT_CHOICE_CONFIRM,
                NavInsID.USE_CASE_STATUS_DISMISS,
            ]
        self._navigate_review(
            text=text,
            validation_instructions=validation_instructions,
            **kwargs
        )

    def expert_splash_navigate(self, **kwargs) -> None:
        """Navigate until the expert mode splash screen."""
        if self._device.is_nano:
            text = "^Next field requires$"
        else:
            text = "^Expert mode$"
        self._navigate_review(text=text, **kwargs)

    def accept_sign_error_risk(self, **kwargs) -> None:
        """Navigate through signing risk warning flow and accept risk."""
        validation_instructions: List[Union[NavIns, BaseNavInsID]] = []
        if self._device.is_nano:
            text = "^Accept risk$"
            validation_instructions = [NavInsID.BOTH_CLICK]
        else:
            text = "^Proceed to Blindsign$"
            validation_instructions = [TezosNavInsID.WARNING_CHOICE_BLINDSIGN]
        self._navigate_review(
            text=text,
            validation_instructions=validation_instructions,
            **kwargs
        )

    def refuse_sign_error_risk(self, **kwargs) -> None:
        """Navigate through signing risk warning flow and accept risk."""
        validation_instructions: List[Union[NavIns, BaseNavInsID]] = []
        if self._device.is_nano:
            text = "^Reject$"
            validation_instructions = [NavInsID.BOTH_CLICK]
        else:
            text = "^Proceed to Blindsign$"
            validation_instructions = [
                TezosNavInsID.WARNING_CHOICE_SAFETY,
                TezosNavInsID.REJECT_CHOICE_CONFIRM,
                NavInsID.USE_CASE_STATUS_DISMISS,
            ]
        self._navigate_review(
            text=text,
            validation_instructions=validation_instructions,
            **kwargs
        )

    def accept_sign_blindsign_risk(self, **kwargs) -> None:
        """Navigate through signing risk warning flow and accept risk."""
        if self._device.is_nano:
            self._navigate_review(
                text="^Accept risk$",
                validation_instructions=[NavInsID.BOTH_CLICK],
                **kwargs
            )
        else:
            self.navigate(
                instructions=[TezosNavInsID.WARNING_CHOICE_BLINDSIGN],
                screen_change_before_first_instruction=True,
                screen_change_after_last_instruction=False,
                **kwargs
            )

    def refuse_sign_blindsign_risk(self, **kwargs) -> None:
        """Navigate through signing risk warning flow and accept risk."""
        if self._device.is_nano:
            self._navigate_review(
                text="^Reject$",
                validation_instructions=[NavInsID.BOTH_CLICK],
                **kwargs
            )
        else:
            self.navigate(
                instructions=[
                    TezosNavInsID.WARNING_CHOICE_SAFETY,
                    TezosNavInsID.REJECT_CHOICE_CONFIRM,
                    NavInsID.USE_CASE_STATUS_DISMISS,
                ],
                screen_change_before_first_instruction=True,
                screen_change_after_last_instruction=False,
                **kwargs
            )

    def skip_sign(self, **kwargs) -> None:
        """Tap on the Skip button and accept to skip.
        Only available for Touch devices, fail otherwise.
        """
        assert not self._device.is_nano, "Skip available only on Touch devices"
        self._navigate_review(
            text="Skip",
            validation_instructions=[
                TezosNavInsID.REVIEW_TX_SKIP,
                TezosNavInsID.SKIP_CHOICE_CONFIRM,
            ],
            **kwargs
        )

    def skip_reject(self, **kwargs) -> None:
        """Tap on the Skip button and reject to skip.
        Only available for Touch devices, fail otherwise.
        """
        assert not self._device.is_nano, "Skip available only on Touch devices"
        self._navigate_review(
            text="Skip",
            validation_instructions=[
                TezosNavInsID.REVIEW_TX_SKIP,
                TezosNavInsID.SKIP_CHOICE_REJECT,
            ],
            **kwargs
        )
