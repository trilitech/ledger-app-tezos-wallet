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
import os
import sys
import time

from contextlib import contextmanager
from enum import Enum
from multiprocessing import Process, Queue
from pathlib import Path
from requests.exceptions import ConnectionError
from typing import Callable, Generator, List, Union

from ragger.backend import SpeculosBackend
from ragger.error import ExceptionRAPDU
from ragger.firmware import Firmware
from ragger.navigator import NavInsID, NanoNavigator

file_path=os.path.abspath(__file__)
dir_path=os.path.dirname(file_path)
sys.path.append(dir_path)

import check_tlv_signature
from apdu import *

MAX_ATTEMPTS = 50

class Screen(str, Enum):
    Home = "home"
    Blind_home = "blind_home"
    Version = "version"
    Settings = "settings"
    Settings_blind_disabled = "settings_blind_signing_disabled"
    Settings_blind_enabled = "settings_blind_signing_enabled"
    Settings_back = "back"
    Quit = "quit"

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
    pass

class TezosAppScreen():

    def __init__(self,
                 backend: SpeculosTezosBackend,
                 commit: str,
                 version: str,
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
        self.commit = bytes.fromhex(commit + "00")
        self.version = bytes.fromhex(version)
        self.golden_run = golden_run
        self.navigator = NanoNavigator(backend, backend.firmware, golden_run)

    def __enter__(self) -> "TezosAppScreen":
        self.backend.__enter__()
        return self

    def __exit__(self, *args):
        self.backend.__exit__(*args)

    def assert_screen(self, screen: Union[str, Screen]) -> None:
        golden_run = self.golden_run and screen not in self.snapshotted
        if golden_run:
            self.snapshotted = self.snapshotted + [screen]
            input(f"Press ENTER to snapshot {screen}")

        path = self.snapshots_dir / f'{screen}.png'
        tmp_path = self.tmp_snapshots_dir / f'{screen}.png'
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
        self.backend.right_click()
        self.assert_screen(Screen.Blind_home)

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

    def quit_blind(self) -> None:
        self.assert_screen(Screen.Home)
        self.backend.right_click()
        self.assert_screen(Screen.Blind_home)
        self.backend.right_click()
        self.assert_screen(Screen.Version)
        self.backend.right_click()
        self.assert_screen(Screen.Settings)
        self.backend.right_click()
        self._quit()

    def navigate_until_text(self, text: str, path: Union[str, Path]) -> None:
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
            assert code == failing_code, f"Expect fail with { code } but fail with { failing_code }"

    def _failing_send(self,
                      send: Callable[[], bytes],
                      text: str,
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
            send=(lambda: self.backend.prompt_public_key(account)),
            navigate=(lambda: self.navigate_until_text("Approve", path)))

    def check_public_key(self,
                         public_key: Union[str, bytes],
                         data: bytes) -> None:
        if isinstance(public_key, str): public_key = bytes.fromhex(public_key)

        assert data == public_key, \
            f"Expected public key {public_key.hex()} but got {data.hex()}"

    def reject_public_key(self,
                          account: Account,
                          path: Union[str, Path]) -> None:

        self._failing_send(
            send=(lambda: self.backend.prompt_public_key(account)),
            text="Reject",
            status_code=StatusCode.REJECT,
            path=path)

    def sign(self,
             account: Account,
             message: Union[str, bytes],
             path: Union[str, Path]) -> bytes:

        return send_and_navigate(
            send=(lambda: self.backend.sign(account, message)),
            navigate=(lambda: self.navigate_until_text("Accept", path)))

    def check_signature(self,
                        signature: Union[str, bytes],
                        data: bytes) -> None:
        if isinstance(signature, str): signature = bytes.fromhex(signature)

        assert data == signature, \
            f"Expected signature {signature.hex()} but got {data.hex()}"

    def check_tlv_signature(self,
                            message: str,
                            pk: str,
                            data: bytes) -> None:
        check_tlv_signature.check_signature(data, pk, message)

    def sign_with_hash(self,
                       account: Account,
                       message: Union[str, bytes],
                       path: Union[str, Path]) -> bytes:

        return send_and_navigate(
            send=(lambda: self.backend.sign_with_hash(account, message)),
            navigate=(lambda: self.navigate_until_text("Accept", path)))

    def check_signature_with_hash(self,
                                 hash: Union[str, bytes],
                                 signature: Union[str, bytes],
                                 data: bytes) -> None:
        if isinstance(hash, str): hash = bytes.fromhex(hash)
        if isinstance(signature, str): signature = bytes.fromhex(signature)

        assert data == hash + signature, \
            f"Expected hash {hash.hex()} and signature {signature.hex()} but got {data.hex()}"

    def check_tlv_signature_with_hash(self,
                                      hash: Union[str, bytes],
                                      message: str,
                                      pk: str,
                                      data: bytes) -> None:
        if isinstance(hash, str): hash = bytes.fromhex(hash)

        assert data.startswith(hash), \
            f"Expected start with hash {hash.hex()} but got {data.hex()}"
        data = data[len(hash):]
        check_tlv_signature.check_signature(data.hex(), pk, message)

    def _failing_signing(self,
                         account: Account,
                         message: Union[str, bytes],
                         with_hash: bool,
                         text: str,
                         status_code: StatusCode,
                         path: Union[str, Path]) -> None:
        sign = self.backend.sign_with_hash if with_hash else self.backend.sign

        self._failing_send(
            send=(lambda: sign(account, message)),
            text=text,
            status_code=status_code,
            path=path)

    def reject_signing(self,
                       account: Account,
                       message: Union[str, bytes],
                       with_hash: bool,
                       path: Union[str, Path]) -> None:
        self._failing_signing(
            account,
            message,
            with_hash,
            text="Reject",
            status_code=StatusCode.REJECT,
            path=path)

    def hard_failing_signing(self,
                             account: Account,
                             message: Union[str, bytes],
                             with_hash: bool,
                             status_code: StatusCode,
                             path: Union[str, Path]) -> None:
        self._failing_signing(account,
                              message,
                              with_hash,
                              "ready for",
                              status_code,
                              path)

    def parsing_error_signing(self,
                              account: Account,
                              message: Union[str, bytes],
                              with_hash: bool,
                              path: Union[str, Path]) -> None:
        self._failing_signing(account,
                              message,
                              with_hash,
                              "Parsing error",
                              StatusCode.PARSE_ERROR,
                              path)

FIRMWARES = [
    Firmware.NANOS,
    Firmware.NANOSP,
    Firmware.NANOX,
]

DEFAULT_SEED = ('zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra')

DEFAULT_ACCOUNT = Account("m/44'/1729'/0'/0'", SIGNATURE_TYPE.ED25519)

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
    parser.add_argument("--commit",
                        type=str,
                        help="Commit",
                        required=True)
    parser.add_argument("--version",
                        type=str,
                        help="Version",
                        required=True)
    parser.add_argument("--display",
                        type=str,
                        default="headless",
                        help="Display")
    parser.add_argument("--golden-run",
                        action='store_const',
                        const=True,
                        default=False,
                        help="Golden run")
    parser.add_argument("app",
                        type=str,
                        help="App")
    args = parser.parse_args()

    firmware = next(fw for fw in FIRMWARES if fw.device == args.device)
    speculos_args = [ "--display", args.display,
                      "--apdu-port", "0",
                      "--api-port", f"{args.port}",
                      "--seed", seed]
    backend = SpeculosTezosBackend(args.app,
                                   firmware,
                                   port=args.port,
                                   args=speculos_args)
    with TezosAppScreen(backend, args.commit, args.version, args.golden_run) as app:
        yield app
