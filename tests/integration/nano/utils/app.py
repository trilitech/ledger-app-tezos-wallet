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
from pathlib import Path
from requests.exceptions import ConnectionError
from typing import Generator, List, Union

from ragger.backend import SpeculosBackend
from ragger.firmware import Firmware

file_path=os.path.abspath(__file__)
dir_path=os.path.dirname(file_path)
sys.path.append(dir_path)

from apdu import *

MAX_ATTEMPTS = 50

class Screen(str, Enum):
    Home = "home"
    Version = "version"
    Settings = "settings"
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

FIRMWARES = [
    Firmware.NANOS,
    Firmware.NANOSP,
    Firmware.NANOX,
]

DEFAULT_SEED = ('zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra')

@contextmanager
def nano_app() -> Generator[TezosAppScreen, None, None]:
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
                      "--seed", DEFAULT_SEED]
    backend = SpeculosTezosBackend(args.app,
                                   firmware,
                                   port=args.port,
                                   args=speculos_args)
    with TezosAppScreen(backend, args.commit, args.version, args.golden_run) as app:
        yield app
