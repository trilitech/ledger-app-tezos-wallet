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

import argparse
import git
import time
from contextlib import contextmanager
from enum import Enum
from multiprocessing import Process, Queue
from pathlib import Path
from requests.exceptions import ConnectionError
from ragger.backend import SpeculosBackend
from ragger.error import ExceptionRAPDU
from ragger.navigator import NavInsID, NanoNavigator
from typing import Callable, Generator, List, Optional, Tuple, Union

from .message import Message
from .account import Account, SIGNATURE_TYPE
from .backend import StatusCode, TezosBackend, APP_KIND

version: Tuple[int, int, int] = (3, 0, 4)

class TezosAPDUChecker:

    def __init__(self, app_kind: APP_KIND):
        self.app_kind = app_kind

    @property
    def commit(self) -> bytes:
        repo = git.Repo(".")
        commit = repo.head.commit.hexsha[:8]
        if repo.is_dirty(): commit += "*"
        return bytes.fromhex(commit.encode('utf-8').hex() + "00")

    @property
    def version_bytes(self) -> bytes:
        return \
            self.app_kind.to_bytes(1, byteorder='big') + \
            version[0].to_bytes(1, byteorder='big') + \
            version[1].to_bytes(1, byteorder='big') + \
            version[2].to_bytes(1, byteorder='big')

    def check_commit(self, commit: bytes) -> None:
        assert commit == self.commit, \
            f"Expected commit {self.commit.hex()} but got {commit.hex()}"

    def check_version(self, version: bytes) -> None:
        assert version == self.version_bytes, \
            f"Expected version {self.version_bytes.hex()} but got {version.hex()}"

    def check_public_key(self,
                         account: Account,
                         public_key: bytes) -> None:
        account.check_public_key(public_key)

    def check_signature(self,
                        account: Account,
                        message: Message,
                        with_hash: bool,
                        data: bytes) -> None:
        if with_hash:
            assert data.startswith(message.hash), \
                f"Expected a starting hash {message.hash.hex()} but got {data.hex()}"

            data = data[len(message.hash):]

        account.check_signature(data, message.bytes)

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

def run_simultaneously(processes: List[Process]) -> None:

    for process in processes:
        process.start()

    for process in processes:
        process.join()
        assert process.exitcode == 0, "Should have terminate successfully"

def send_and_navigate(send: Callable[[], bytes], navigate: Callable[[], None]) -> bytes:

    def _send(result_queue: Queue) -> None:
        res = send()
        result_queue.put(res)

    result_queue: Queue = Queue()
    send_process = Process(target=_send, args=(result_queue,))
    navigate_process = Process(target=navigate)
    run_simultaneously([navigate_process, send_process])
    return result_queue.get()

class SpeculosTezosBackend(TezosBackend, SpeculosBackend):

    # speculos can be slow to start up in a slow environment.
    # Here, we expect a little more
    def __enter__(self) -> "SpeculosTezosBackend":
        try:
            super().__enter__()
            return self
        except Exception as e:
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
    Home = "home"
    Version = "version"
    Settings = "settings"
    Settings_expert_mode_disabled = "settings_expert_mode_disabled"
    Settings_expert_mode_enabled = "settings_expert_mode_enabled"
    Settings_blindsign_large_tx = "settings_blindsign_large_tx"
    Settings_blindsign_on = "settings_blindsign_on"
    Settings_blindsign_off = "settings_blindsign_off"
    Settings_back = "back"
    Quit = "quit"

class Screen_text(str, Enum):
    Home = "Application"
    Public_key_approve = "Approve"
    Public_key_reject = "Reject"
    Sign_accept = "Accept"
    Sign_reject = "Reject"
    Blind_switch = "Accept risk"
    Back_home = "Home"

class TezosAppScreen():

    def __init__(self,
                 backend: SpeculosTezosBackend,
                 app_kind: APP_KIND,
                 golden_run: bool):
        self.backend = backend
        self.checker = TezosAPDUChecker(app_kind)

        self.path: Path = Path(__file__).resolve().parent.parent
        self.snapshots_dir: Path = self.path / "snapshots" / backend.firmware.name
        self.tmp_snapshots_dir: Path = self.path / "snapshots-tmp" / backend.firmware.name
        if not self.snapshots_dir.is_dir() and golden_run:
            self.snapshots_dir.mkdir(parents=True)
        if not self.tmp_snapshots_dir.is_dir():
            self.tmp_snapshots_dir.mkdir(parents=True)
        self.snapshotted: List[str] = []

        self.golden_run = golden_run
        self.navigator = NanoNavigator(backend, backend.firmware, golden_run)

    def __enter__(self) -> "TezosAppScreen":
        with_retry(self.backend.__enter__, attempts=3)
        return self

    def __exit__(self, *args):
        self.backend.__exit__(*args)

    def assert_screen(self, screen: Union[str, Screen], path: Optional[Union[str, Path]] = None) -> None:
        golden_run = self.golden_run and screen not in self.snapshotted
        if golden_run:
            self.snapshotted = self.snapshotted + [screen]
            input(f"Press ENTER to snapshot {screen}")

        snapshots_dir = self.snapshots_dir if path is None else self.snapshots_dir / path
        tmp_snapshots_dir = self.tmp_snapshots_dir if path is None else self.tmp_snapshots_dir / path

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
        self.backend._last_screenshot = path

    def setup_expert_mode(self) -> None:
        self.assert_screen(Screen.Home)
        self.backend.right_click()
        self.assert_screen(Screen.Version)
        self.backend.right_click()
        self.assert_screen(Screen.Settings)
        self.backend.both_click()
        self.assert_screen(Screen.Settings_expert_mode_disabled)
        self.backend.both_click()
        self.assert_screen(Screen.Settings_expert_mode_enabled)
        self.backend.left_click()
        self.assert_screen(Screen.Settings_back)
        self.backend.both_click()
        self.assert_screen(Screen.Home)

    def setup_blindsign_off(self) -> None:
        self.assert_screen(Screen.Home)
        self.backend.right_click()
        self.assert_screen(Screen.Version)
        self.backend.right_click()
        self.assert_screen(Screen.Settings)
        self.backend.both_click()
        # expert_mode screen
        self.backend.right_click()
        self.assert_screen(Screen.Settings_blindsign_large_tx)
        self.backend.both_click()
        self.assert_screen(Screen.Settings_blindsign_on)
        self.backend.both_click()
        self.assert_screen(Screen.Settings_blindsign_off)
        self.backend.right_click()
        self.assert_screen(Screen.Settings_back)
        self.backend.both_click()
        self.assert_screen(Screen.Home)

    def _quit(self) -> None:
        self.assert_screen(Screen.Quit)
        try:
            self.backend.both_click()
            assert False, "Must have lost connection with speculos"
        except ConnectionError:
            pass

    def quit(self) -> None:
        self.assert_screen(Screen.Home)
        self.backend.right_click()
        self.assert_screen(Screen.Version)
        self.backend.right_click()
        self.assert_screen(Screen.Settings)
        self.backend.right_click()
        self._quit()

    def navigate_until_text(self, text: Screen_text, path: Union[str, Path]) -> None:
        if isinstance(path, str): path = Path(path)
        self.navigator.\
            navigate_until_text_and_compare(NavInsID.RIGHT_CLICK,
                                            [NavInsID.BOTH_CLICK],
                                            text,
                                            path=self.path,
                                            test_case_name=path,
                                            screen_change_after_last_instruction=False)

    @contextmanager
    def expect_apdu_failure(self, code: StatusCode) -> Generator[None, None, None]:
        try:
            yield
            assert False, f"Expect fail with { code } but succeed"
        except ExceptionRAPDU as e:
            failing_code = StatusCode(e.status)
            assert code == failing_code, \
                f"Expect fail with { code.name } but fail with { failing_code.name }"

    def _failing_send(self,
                      send: Callable[[], bytes],
                      text: Screen_text,
                      status_code: StatusCode,
                      path: Union[str, Path]) -> None:
        def expected_failure_send() -> bytes:
            with self.expect_apdu_failure(status_code):
                send()
            return b''

        send_and_navigate(
            send=expected_failure_send,
            navigate=(lambda: self.navigate_until_text(text, path)))

    def provide_public_key(self,
                          account: Account,
                          path: Union[str, Path]) -> bytes:

        return send_and_navigate(
            send=(lambda: self.backend.get_public_key(account, with_prompt=True)),
            navigate=(lambda: self.navigate_until_text(Screen_text.Public_key_approve, path)))

    def reject_public_key(self,
                          account: Account,
                          path: Union[str, Path]) -> None:

        self._failing_send(
            send=(lambda: self.backend.get_public_key(account, with_prompt=True)),
            text=Screen_text.Public_key_reject,
            status_code=StatusCode.REJECT,
            path=path)

    def sign(self,
             account: Account,
             message: Message,
             with_hash: bool,
             path: Union[str, Path]) -> bytes:

        return send_and_navigate(
            send=(lambda: self.backend.sign(account, message, with_hash)),
            navigate=(lambda: self.navigate_until_text(Screen_text.Sign_accept, path)))

    def blind_sign(self,
                   account: Account,
                   message: Message,
                   with_hash: bool,
                   path: Union[str, Path]) -> bytes:

        if isinstance(path, str): path = Path(path)

        def navigate() -> None:
            self.navigate_until_text(Screen_text.Blind_switch, path / "clear")
            self.navigate_until_text(Screen_text.Sign_accept, path / "blind")

        return send_and_navigate(
            send=(lambda: self.backend.sign(account, message, with_hash)),
            navigate=navigate)

    def _failing_signing(self,
                         account: Account,
                         message: Message,
                         with_hash: bool,
                         text: Screen_text,
                         status_code: StatusCode,
                         path: Union[str, Path]) -> None:

        self._failing_send(
            send=(lambda: self.backend.sign(account, message, with_hash)),
            text=text,
            status_code=status_code,
            path=path)

    def reject_signing(self,
                       account: Account,
                       message: Message,
                       with_hash: bool,
                       path: Union[str, Path]) -> None:
        self._failing_signing(
            account,
            message,
            with_hash,
            text=Screen_text.Sign_reject,
            status_code=StatusCode.REJECT,
            path=path)

    def hard_failing_signing(self,
                             account: Account,
                             message: Message,
                             with_hash: bool,
                             status_code: StatusCode,
                             path: Union[str, Path]) -> None:
        self._failing_signing(account,
                              message,
                              with_hash,
                              Screen_text.Home,
                              status_code,
                              path)

    def parsing_error_signing(self,
                              account: Account,
                              message: Message,
                              with_hash: bool,
                              path: Union[str, Path]) -> None:
        self._failing_signing(account,
                              message,
                              with_hash,
                              Screen_text.Sign_reject,
                              StatusCode.PARSE_ERROR,
                              path)

DEFAULT_SEED = ('zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra')

DEFAULT_ACCOUNT = Account("m/44'/1729'/0'/0'",
                          SIGNATURE_TYPE.ED25519,
                          "edpkuXX2VdkdXzkN11oLCb8Aurdo1BTAtQiK8ZY9UPj2YMt3AHEpcY")
