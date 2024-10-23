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

from contextlib import contextmanager
from enum import Enum
from io import BytesIO
from multiprocessing import Process, Queue
from pathlib import Path
import time
from typing import Callable, Generator, List, Optional, Tuple, Union

import requests
import git
from ragger.backend import SpeculosBackend
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID, NanoNavigator

from .message import Message
from .account import Account, SigType
from .backend import StatusCode, TezosBackend, AppKind

version: Tuple[int, int, int] = (3, 0, 5)

class TezosAPDUChecker:
    """Helper to check APDU received."""

    app_kind: AppKind

    def __init__(self, app_kind: AppKind):
        self.app_kind = app_kind

    @property
    def commit(self) -> bytes:
        """Current commit."""
        repo = git.Repo(".")
        commit = repo.head.commit.hexsha[:8]
        if repo.is_dirty():
            commit += "*"
        return bytes.fromhex(commit.encode('utf-8').hex() + "00")

    @property
    def version_bytes(self) -> bytes:
        """Current version in bytes."""
        return \
            self.app_kind.to_bytes(1, byteorder='big') + \
            version[0].to_bytes(1, byteorder='big') + \
            version[1].to_bytes(1, byteorder='big') + \
            version[2].to_bytes(1, byteorder='big')

    def check_commit(self, commit: bytes) -> None:
        """Check if the commit is the current commit."""
        assert commit == self.commit, \
            f"Expected commit {self.commit.hex()} but got {commit.hex()}"

    def check_version(self, raw_version: bytes) -> None:
        """Check if the version is the current version."""
        assert raw_version == self.version_bytes, \
            f"Expected version {self.version_bytes.hex()} but got {raw_version.hex()}"

    def check_public_key(self,
                         account: Account,
                         public_key: bytes) -> None:
        """Check that public_key is the account public key."""
        account.check_public_key(public_key)

    def check_signature(self,
                        account: Account,
                        message: Message,
                        with_hash: bool,
                        data: bytes) -> None:
        """Check that data is a signature of message from account."""
        if with_hash:
            assert data.startswith(message.hash), \
                f"Expected a starting hash {message.hash.hex()} but got {data.hex()}"

            data = data[len(message.hash):]

        account.check_signature(data, bytes(message))

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

def run_simultaneously(processes: List[Process]) -> None:
    """Executes multiples process at the same time."""

    for process in processes:
        process.start()

    for process in processes:
        process.join()
        assert process.exitcode == 0, "Should have terminate successfully"

def send_and_navigate(send: Callable[[], bytes], navigate: Callable[[], None]) -> bytes:
    """Sends APDU and navigates."""

    def _send(result_queue: Queue) -> None:
        res = send()
        result_queue.put(res)

    result_queue: Queue = Queue()
    send_process = Process(target=_send, args=(result_queue,))
    navigate_process = Process(target=navigate)
    run_simultaneously([navigate_process, send_process])
    return result_queue.get()

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
    checker: TezosAPDUChecker
    path: Path
    snapshots_dir: Path
    tmp_snapshots_dir: Path
    snapshotted: List[str]
    golden_run: bool
    navigator: NanoNavigator

    def __init__(self,
                 backend: SpeculosTezosBackend,
                 app_kind: AppKind,
                 golden_run: bool):
        self.backend = backend
        self.checker = TezosAPDUChecker(app_kind)

        self.path = Path(__file__).resolve().parent.parent
        self.snapshots_dir = self.path / "snapshots" / backend.firmware.name
        self.tmp_snapshots_dir = self.path / "snapshots-tmp" / backend.firmware.name
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

    def navigate_until_text(self, text: ScreenText, path: Union[str, Path]) -> None:
        """Click right until the expected text is displayed, then both click."""
        if isinstance(path, str):
            path = Path(path)
        self.navigator.\
            navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                            [NavInsID.BOTH_CLICK],
                                            text,
                                            path=self.path,
                                            test_case_name=path,
                                            screen_change_after_last_instruction=False)

    @contextmanager
    def expect_apdu_failure(self, code: StatusCode) -> Generator[None, None, None]:
        """Expect the body to fail with code."""
        try:
            yield
            assert False, f"Expect fail with { code } but succeed"
        except ExceptionRAPDU as e:
            failing_code = StatusCode(e.status)
            assert code == failing_code, \
                f"Expect fail with { code.name } but fail with { failing_code.name }"

    def _failing_send(self,
                      send: Callable[[], bytes],
                      navigate: Callable[[], None],
                      status_code: StatusCode) -> None:
        """Expect that send and navigates fails with status_code."""
        def expected_failure_send() -> bytes:
            with self.expect_apdu_failure(status_code):
                send()
            return b''

        send_and_navigate(
            send=expected_failure_send,
            navigate=navigate)

    def provide_public_key(self,
                          account: Account,
                          path: Union[str, Path]) -> bytes:
        """Get the account's public key from the app after approving it."""
        return send_and_navigate(
            send=lambda: self.backend.get_public_key(account, with_prompt=True),
            navigate=lambda: self.navigate_until_text(ScreenText.PUBLIC_KEY_APPROVE, path))

    def reject_public_key(self,
                          account: Account,
                          path: Union[str, Path]) -> None:
        """Reject the account's public key."""
        self._failing_send(
            send=(lambda: self.backend.get_public_key(account, with_prompt=True)),
            navigate=(lambda: self.navigate_until_text(
                ScreenText.PUBLIC_KEY_REJECT,
                path)),
            status_code=StatusCode.REJECT)

    def _sign(self,
              account: Account,
              message: Message,
              with_hash: bool,
              navigate: Callable[[], None]) -> bytes:
        """Requests to sign the message with account and navigates."""
        return send_and_navigate(
            send=(lambda: self.backend.sign(account, message, with_hash)),
            navigate=navigate)

    def sign(self,
             account: Account,
             message: Message,
             with_hash: bool,
             path: Union[str, Path]) -> bytes:
        """Sign the message with account."""
        return self._sign(
            account,
            message,
            with_hash,
            navigate=lambda: self.navigate_until_text(ScreenText.SIGN_ACCEPT, path))

    def blind_sign(self,
                   account: Account,
                   message: Message,
                   with_hash: bool,
                   path: Union[str, Path]) -> bytes:
        """Blindsign the message with account."""
        if isinstance(path, str):
            path = Path(path)

        def navigate() -> None:
            self.navigate_until_text(ScreenText.ACCEPT_RISK, path / "clear")
            self.navigate_until_text(ScreenText.SIGN_ACCEPT, path / "blind")

        return send_and_navigate(
            send=(lambda: self.backend.sign(account, message, with_hash)),
            navigate=navigate)

    def _failing_signing(self,
                         account: Account,
                         message: Message,
                         with_hash: bool,
                         navigate: Callable[[], None],
                         status_code: StatusCode) -> None:
        """Expect requests signing and navigate fails with status_code."""
        self._failing_send(
            send=(lambda: self.backend.sign(account, message, with_hash)),
            navigate=navigate,
            status_code=status_code)

    def reject_signing(self,
                       account: Account,
                       message: Message,
                       with_hash: bool,
                       path: Union[str, Path]) -> None:
        """Request and reject signing the message."""
        self._failing_signing(
            account,
            message,
            with_hash,
            navigate=(lambda: self.navigate_until_text(
                ScreenText.SIGN_REJECT,
                path)),
            status_code=StatusCode.REJECT)

    def hard_failing_signing(self,
                             account: Account,
                             message: Message,
                             with_hash: bool,
                             status_code: StatusCode,
                             path: Union[str, Path]) -> None:
        """Expect the signing request to hard fail."""
        self._failing_signing(account,
                              message,
                              with_hash,
                              navigate=(lambda: self.navigate_until_text(
                                  ScreenText.HOME,
                                  path)),
                              status_code=status_code)

    def parsing_error_signing(self,
                              account: Account,
                              message: Message,
                              with_hash: bool,
                              path: Union[str, Path]) -> None:
        """Expect the signing request to fail with parsing error."""
        self._failing_signing(account,
                              message,
                              with_hash,
                              navigate=(lambda: self.navigate_until_text(
                                  ScreenText.SIGN_REJECT,
                                  path)),
                              status_code=StatusCode.PARSE_ERROR)

DEFAULT_SEED = 'zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra'

DEFAULT_ACCOUNT = Account("m/44'/1729'/0'/0'",
                          SigType.ED25519,
                          "edpkuXX2VdkdXzkN11oLCb8Aurdo1BTAtQiK8ZY9UPj2YMt3AHEpcY")
