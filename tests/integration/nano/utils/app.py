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
import base58
import git
import os
import sys
import time

from contextlib import contextmanager
from enum import Enum
from multiprocessing import Process, Queue
from pathlib import Path
from requests.exceptions import ConnectionError
from typing import Callable, Generator, List, Optional, Tuple, Union

from ragger.backend import SpeculosBackend
from ragger.error import ExceptionRAPDU
from ragger.firmware import Firmware
from ragger.navigator import NavInsID, NanoNavigator

file_path=os.path.abspath(__file__)
dir_path=os.path.dirname(file_path)
sys.path.append(dir_path)

from backend import *
from message import Message

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
    navigate_process = Process(target=navigate)
    send_process = Process(target=_send, args=(result_queue,))

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

version: Tuple[int, int, int] = (3, 0, 0)

class Screen(str, Enum):
    Home = "home"
    Blind_home = "blind_home"
    Version = "version"
    Settings = "settings"
    Settings_blind_disabled = "settings_blind_signing_disabled"
    Settings_blind_enabled = "settings_blind_signing_enabled"
    Settings_back = "back"
    Quit = "quit"

class Screen_text(str, Enum):
    Home = "Application"
    Public_key_approve = "Approve"
    Public_key_reject = "Reject"
    Sign_accept = "Accept"
    Sign_reject = "Reject"
    Blind_switch = "Switch to"
    Back_home = "Home"

class TezosAppScreen():

    def __init__(self,
                 backend: SpeculosTezosBackend,
                 app_kind: APP_KIND,
                 golden_run: bool):
        self.backend = backend

        self.path: Path = Path(__file__).resolve().parent.parent
        self.snapshots_dir: Path = self.path / "snapshots" / backend.firmware.name
        self.tmp_snapshots_dir: Path = self.path / "snapshots-tmp" / backend.firmware.name
        if not self.snapshots_dir.is_dir() and golden_run:
            self.snapshots_dir.mkdir(parents=True)
        if not self.tmp_snapshots_dir.is_dir():
            self.tmp_snapshots_dir.mkdir(parents=True)
        self.snapshotted: List[str] = []

        repo = git.Repo(".")
        commit = repo.head.commit.hexsha[:8]
        if repo.is_dirty(): commit += "*"
        self.commit = bytes.fromhex(commit.encode('utf-8').hex() + "00")
        self.version = \
            app_kind.to_bytes(1, byteorder='big') + \
            version[0].to_bytes(1, byteorder='big') + \
            version[1].to_bytes(1, byteorder='big') + \
            version[2].to_bytes(1, byteorder='big')

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

    def setup_blind_signing(self) -> None:
        self.assert_screen(Screen.Home)
        self.backend.right_click()
        self.assert_screen(Screen.Version)
        self.backend.right_click()
        self.assert_screen(Screen.Settings)
        self.backend.both_click()
        self.assert_screen(Screen.Settings_blind_disabled)
        self.backend.both_click()
        self.assert_screen(Screen.Settings_blind_enabled)
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

    def check_public_key(self,
                         account: Account,
                         data: bytes) -> None:

        account.check_public_key(data)

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

    def check_signature(self,
                        account: Account,
                        message: Message,
                        signature: bytes) -> None:
        account.check_signature(signature, message.bytes)

    def check_signature_with_hash(self,
                                  account: Account,
                                  message: Message,
                                  data: bytes) -> None:
        assert data.startswith(message.hash), \
            f"Expected a starting hash {message.hash.hex()} but got {data.hex()}"

        signature = data[len(message.hash):]

        self.check_signature(account, message, signature)

    def blind_sign(self,
                   account: Account,
                   message: Message,
                   with_hash: bool,
                   path: Union[str, Path]) -> bytes:

        if isinstance(path, str): path = Path(path)

        self.setup_blind_signing()

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
                              Screen_text.Back_home,
                              StatusCode.PARSE_ERROR,
                              path)

FIRMWARES = [
    Firmware.NANOS,
    Firmware.NANOSP,
    Firmware.NANOX,
]

DEFAULT_SEED = ('zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra')

DEFAULT_ACCOUNT = Account("m/44'/1729'/0'/0'",
                          SIGNATURE_TYPE.ED25519,
                          "edpkuXX2VdkdXzkN11oLCb8Aurdo1BTAtQiK8ZY9UPj2YMt3AHEpcY")

@contextmanager
def nano_app(seed: str = DEFAULT_SEED) -> Generator[TezosAppScreen, None, None]:
    parser = argparse.ArgumentParser(description="Launch a nano speculos backend for the tezos app")
    parser.add_argument("-d", "--device",
                        type=str,
                        choices=list(map(lambda fw: fw.device, FIRMWARES)),
                        help="Device type: nanos | nanosp | nanox",
                        required=True)
    parser.add_argument("-p", "--port",
                        type=int,
                        default=5000,
                        help="Port")
    parser.add_argument("--golden-run",
                        action='store_const',
                        const=True,
                        default=False,
                        help="Golden run")
    parser.add_argument("--app",
                        type=str,
                        help="App",
                        required=True)
    args, unknown_args = parser.parse_known_args()

    firmware = next(fw for fw in FIRMWARES if fw.device == args.device)
    speculos_args = [
        "--api-port", f"{args.port}",
        "--seed", seed
    ] + unknown_args
    backend = SpeculosTezosBackend(args.app,
                                   firmware,
                                   port=args.port,
                                   args=speculos_args)
    with TezosAppScreen(backend, APP_KIND.WALLET, args.golden_run) as app:
        yield app
