# Copyright 2024 Trilitech <contact@trili.tech>
# Copyright 2024 Functori <contact@functori.com>

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Conftest base on `ragger` conftest."""


from pathlib import Path
from typing import Dict, Generator, List, Union

import pytest
from ragger.firmware import Firmware
from ragger.navigator import Navigator, NanoNavigator, TouchNavigator

from utils.account import Account, DEFAULT_ACCOUNT, DEFAULT_SEED
from utils.backend import TezosBackend, SpeculosTezosBackend
from utils.navigator import TezosNavigator

FIRMWARES: List[Firmware] = [
    Firmware.NANOS,
    Firmware.NANOSP,
    Firmware.NANOX,
    Firmware.STAX,
    Firmware.FLEX
]

DEVICES: List[str] = list(map(lambda fw: fw.device, FIRMWARES))

def pytest_addoption(parser):
    """Register argparse-style options for pytest."""
    parser.addoption("-D", "--device",
                     type=str,
                     choices=DEVICES,
                     help="Device type: nanos | nanosp | nanox | stax | flex",
                     required=True)
    parser.addoption("-P", "--port",
                     type=int,
                     default=5000,
                     help="Port")
    parser.addoption("--display",
                     action="store_const",
                     const=True,
                     default=False,
                     help="Emulate the app")
    parser.addoption("--golden-run",
                     action="store_const",
                     const=True,
                     default=False,
                     help="Golden run")
    parser.addoption("--log-dir",
                     type=Path,
                     help="Log dir")
    parser.addoption("--speculos-args",
                     type=str,
                     help="Speculos arguments")
    parser.addoption("--app",
                     type=str,
                     help="App",
                     required=True)

global_log_dir: Union[Path, None] = None

def pytest_configure(config):
    """Configure pytest."""
    global global_log_dir
    log_dir = config.getoption("log_dir")
    if log_dir is not None:
        global_log_dir = Path(log_dir)

logs : Dict[str, List[pytest.TestReport]] = {}

@pytest.hookimpl(tryfirst=True)
def pytest_runtest_logstart(location):
    """Called at the start of running the runtest protocol for a single item."""
    logs[location[2]] = []

@pytest.hookimpl(tryfirst=True)
def pytest_runtest_logreport(report):
    """Called at the end of running the runtest protocol for a single test."""
    logs[report.head_line].append(report)

@pytest.hookimpl(tryfirst=True)
def pytest_runtest_logfinish(nodeid, location):
    """Called at the end of running the runtest protocol for a single item."""
    if global_log_dir is not None:
        log_dir = Path(nodeid.split(".py")[0])
        # Remove `tests/integration/nano/`
        log_dir = Path(*log_dir.parts[3:])
        log_dir = global_log_dir / log_dir
        log_dir.mkdir(parents=True, exist_ok=True)
        head_line = location[2]
        log_file = log_dir / f"{head_line}.log"
        with open(log_file, 'w', encoding="utf-8") as writer:
            for report in logs[head_line]:
                writer.write(f"============================== {report.when.capitalize()} {report.outcome} ==============================\n")
                writer.write(f"{report.longreprtext}\n")
                for section in report.sections:
                    if section[0].endswith(report.when):
                        writer.write(f"------------------------------ {section[0]} ------------------------------\n")
                        writer.write(f"{section[1]}\n")
                        writer.write("\n")
                writer.write("\n")

@pytest.fixture(scope="session")
def firmware(pytestconfig) -> Firmware :
    """Get `firware` for pytest."""
    device = pytestconfig.getoption("device")
    return next(fw for fw in FIRMWARES if fw.device == device)

@pytest.fixture(scope="session")
def port(pytestconfig, worker_id) -> int :
    """Get `port` for pytest."""
    if worker_id == "master":
        return pytestconfig.getoption("port")
    # worker_id = gw0, gw1, ...
    return 5000 + int(worker_id[2:])

@pytest.fixture(scope="session")
def display(pytestconfig) -> bool :
    """Get `display` for pytest."""
    return pytestconfig.getoption("display")

@pytest.fixture(scope="session")
def golden_run(pytestconfig) -> bool:
    """Get `golden_run` for pytest."""
    return pytestconfig.getoption("golden_run")

@pytest.fixture(scope="session")
def app_path(pytestconfig) -> Path:
    """Get `app_path` for pytest."""
    return Path(pytestconfig.getoption("app"))

@pytest.fixture(scope="session")
def speculos_args(pytestconfig) -> List[str]:
    """Get `app_path` for pytest."""
    speculos_args = pytestconfig.getoption("speculos_args")
    if speculos_args is None:
        return []
    return speculos_args.split()

@pytest.fixture(scope="function")
def seed(request) -> str:
    """Get `seed` for pytest."""
    param = getattr(request, "param", None)
    return param.get("seed", DEFAULT_SEED) if param else DEFAULT_SEED

@pytest.fixture(scope="function")
def account(request) -> Account:
    """Get `account` for pytest."""
    param = getattr(request, "param", None)
    return param.get("account", DEFAULT_ACCOUNT) if param else DEFAULT_ACCOUNT

@pytest.fixture(scope="function")
def backend(app_path: Path,
            firmware: Firmware,
            port: int,
            display: bool,
            seed: str,
            speculos_args: List[str]) -> Generator[TezosBackend, None, None]:
    """Get `backend` for pytest."""

    if display:
        speculos_args += ["--display", "qt"]

    speculos_args += [
        "--api-port", f"{ port }",
        "--apdu-port", "0",
        "--seed", seed
    ]

    backend = SpeculosTezosBackend(app_path,
                                   firmware,
                                   args=speculos_args)

    with backend as b:
        yield b

@pytest.fixture(scope="function")
def tezos_navigator(
        backend: TezosBackend,
        firmware: Firmware,
        golden_run: bool
) -> TezosNavigator:
    """Get `navigator` for pytest."""
    if firmware.is_nano:
        navigator: Navigator = NanoNavigator(backend, firmware, golden_run)
    else:
        navigator = TouchNavigator(backend, firmware, golden_run)
    return TezosNavigator(backend, navigator)

@pytest.fixture(scope="function")
def snapshot_dir(request) -> Path :
    """Get the test snapshot location."""
    test_file_path = Path(request.fspath)
    file_name = test_file_path.stem
    test_name = request.node.name
    # Get test directory from the root
    test_file_snapshot_dir = Path(*test_file_path.parts[len(Path(__file__).parts)-1:-1])
    return test_file_snapshot_dir / file_name / test_name

def requires_device(device):
    """Wrapper to run the pytest test only with the provided device."""
    return pytest.mark.skipif(
        f"config.getvalue('device') != '{ device }'",
        reason=f"Test requires device to be { device }."
    )
