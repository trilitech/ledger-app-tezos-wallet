import pytest

from pathlib import Path
from ragger.firmware import Firmware
from typing import Dict, Callable, Generator, List, Optional, Tuple, Union

from utils.app import TezosAppScreen, SpeculosTezosBackend, DEFAULT_SEED
from utils.backend import TezosBackend, APP_KIND

FIRMWARES: List[Firmware] = [
    Firmware.NANOS,
    Firmware.NANOSP,
    Firmware.NANOX,
]

DEVICES: List[str] = list(map(lambda fw: fw.device, FIRMWARES))

def pytest_addoption(parser):
    parser.addoption("-D", "--device",
                     type=str,
                     choices=DEVICES,
                     help="Device type: nanos | nanosp | nanox",
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
    global global_log_dir
    log_dir = config.getoption("log_dir")
    if log_dir is not None:
        global_log_dir = Path(log_dir)

logs : Dict[str, List[pytest.TestReport]] = {}

@pytest.hookimpl(tryfirst=True)
def pytest_runtest_logstart(nodeid, location):
    logs[location[2]] = []

@pytest.hookimpl(tryfirst=True)
def pytest_runtest_logreport(report):
    logs[report.head_line].append(report)

@pytest.hookimpl(tryfirst=True)
def pytest_runtest_logfinish(nodeid, location):
    if global_log_dir is not None:
        log_dir = global_log_dir / Path(location[0]).stem
        log_dir.mkdir(parents=True, exist_ok=True)
        head_line = location[2]
        log_file = log_dir / f"{head_line.replace(' ', '_')}.log"
        with open(log_file, 'w') as writer:
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
    device = pytestconfig.getoption("device")
    return next(fw for fw in FIRMWARES if fw.device == device)

@pytest.fixture(scope="session")
def port(pytestconfig, request, worker_id) -> int :
    if worker_id == "master":
        return pytestconfig.getoption("port")
    # worker_id = gw0, gw1, ...
    return 5000 + int(worker_id[2:])

@pytest.fixture(scope="session")
def display(pytestconfig) -> bool :
    return pytestconfig.getoption("display")

@pytest.fixture(scope="session")
def golden_run(pytestconfig) -> bool:
    return pytestconfig.getoption("golden_run")

@pytest.fixture(scope="session")
def app_path(pytestconfig) -> Path:
    return Path(pytestconfig.getoption("app"))

@pytest.fixture(scope="session")
def speculos_args(pytestconfig) -> List[str]:
    speculos_args = pytestconfig.getoption("speculos_args")
    if speculos_args is None:
        return []
    return speculos_args.split()

@pytest.fixture(scope="function")
def seed(request) -> str:
    param = getattr(request, "param", None)
    return param.get("seed", DEFAULT_SEED) if param else DEFAULT_SEED

@pytest.fixture(scope="function")
def backend(app_path: Path,
            firmware: Firmware,
            port: int,
            display: bool,
            seed: str,
            speculos_args: List[str]) -> Generator[SpeculosTezosBackend, None, None]:

    if display:
        speculos_args += ["--display", "qt"]

    speculos_args += [
        "--api-port", f"{ port }",
        "--apdu-port", "0",
        "--seed", seed
    ]

    backend = SpeculosTezosBackend(app_path,
                                   firmware,
                                   port=port,
                                   args=speculos_args)

    with backend as b:
        yield b

@pytest.fixture(scope="function")
def app(backend: SpeculosTezosBackend, golden_run: bool):
    return TezosAppScreen(backend, APP_KIND.WALLET, golden_run)

def requires_device(device):
    return pytest.mark.skipif(
        f"config.getvalue('device') != '{ device }'",
        reason=f"Test requires device to be { device }."
    )
