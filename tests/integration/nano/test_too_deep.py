#!/usr/bin/env python3
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

from utils.apdu import *
from utils.app import *

# Expression: {{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{42}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        expression="0502000000f702000000f202000000ed02000000e802000000e302000000de02000000d902000000d402000000cf02000000ca02000000c502000000c002000000bb02000000b602000000b102000000ac02000000a702000000a2020000009d02000000980200000093020000008e02000000890200000084020000007f020000007a02000000750200000070020000006b02000000660200000061020000005c02000000570200000052020000004d02000000480200000043020000003e02000000390200000034020000002f020000002a02000000250200000020020000001b02000000160200000011020000000c02000000070200000002002a"

        if app.backend.firmware.device == "nanos":
            def send() -> None:
                with app.expect_apdu_failure(StatusCode.PARSE_ERROR):
                    app.backend.sign(DEFAULT_ACCOUNT, expression, with_hash=True)
            def assert_screen_i(i):
                app.assert_screen(f"{str(i).zfill(5)}", path=test_name)

            send_process = Process(target=send)
            send_process.start()

            for i in range(5):
                assert_screen_i(i)
                app.backend.right_click()

            assert_screen_i(i+1)
            app.backend.both_click()

            send_process.join()
            assert send_process.exitcode == 0, "Should have terminate successfully"
        else:
            app.parsing_error_signing(DEFAULT_ACCOUNT,
                                      expression,
                                      with_hash=True,
                                      path=test_name)

        app.quit()
