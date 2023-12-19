import pytest

from pathlib import Path
from ragger.firmware import Firmware
from typing import Callable, Generator, List, Optional, Tuple, Union

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
    parser.addoption("--app",
                     type=str,
                     help="App",
                     required=True)

@pytest.fixture(scope="session")
def firmware(pytestconfig) -> Firmware :
    device = pytestconfig.getoption("device")
    return next(fw for fw in FIRMWARES if fw.device == device)

@pytest.fixture(scope="session")
def port(pytestconfig) -> int :
    return pytestconfig.getoption("port")

@pytest.fixture(scope="session")
def display(pytestconfig) -> bool :
    return pytestconfig.getoption("display")

@pytest.fixture(scope="session")
def golden_run(pytestconfig) -> bool:
    return pytestconfig.getoption("golden_run")

@pytest.fixture(scope="session")
def app_path(pytestconfig) -> Path:
    return Path(pytestconfig.getoption("app"))

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
            speculos_args: List[str] = []) -> Generator[SpeculosTezosBackend, None, None]:

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
