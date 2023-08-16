# Copyright 2023 Trilitech <contact@trili.tech>

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import time

from ragger.backend import SpeculosBackend
from ragger.firmware import Firmware
from ragger.firmware.stax.screen import MetaScreen
from ragger.firmware.stax.use_cases import UseCaseHome, UseCaseSettings, UseCaseAddressConfirmation

MAX_ATTEMPTS = 100
# tezos logo is read as '5' by OCR
TEZOS_LOGO = "5"
# X in circle is read as 'cS'
ICON_CROSS = "cS"

HOME_TEXT = [TEZOS_LOGO, "Tezos", "This app enables signing", "transactions on the Tezos", "network.", "Quit app"]
INFO_TEXT = ['Tezos wallet', 'Version', '2.3.2', 'Developer', 'Tezos', 'Copyright', '(c) 2023 <Tezos>']

def with_retry(f, attempts=MAX_ATTEMPTS):
    while True:
        try:
            return f()
        except AssertionError as e:
            if attempts <= 0:
                print("- with_retry: attempts exhausted -")
                raise e
        attempts -= 1
        time.sleep(0.5)

def ocr_convert(text):
    return text.replace('6', 'G')

class TezosAppScreen(metaclass=MetaScreen):
    use_case_welcome = UseCaseHome
    use_case_info = UseCaseSettings
    use_case_provide_pk = UseCaseAddressConfirmation

    __backend = None

    def __init__(self, backend, firmware):
        self.__backend = backend

    def assert_screen(self, expected_content, attempts=MAX_ATTEMPTS):
        def check():
            content = [e["text"] for e in self.__backend.get_current_screen_content()["events"]]
            print(f'- Found {content}, expected {expected_content} -')
            assert len(content) >= len(expected_content), f"screen {content} shorter than {expected_content}"

            for (expected, actual) in zip(expected_content, content):
                expected = ocr_convert(expected)
                assert expected in actual, f"Expected '{expected}' found '{actual}'"

        with_retry(check, attempts)

def stax_app() -> TezosAppScreen:
    port = os.environ["PORT"]
    backend = SpeculosBackend("__unused__", Firmware.STAX, port = port)
    return TezosAppScreen(backend, Firmware.STAX)
