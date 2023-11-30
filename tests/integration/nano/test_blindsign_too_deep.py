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

from utils.app import *
from utils.backend import *

# Expression: {{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{42}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}

if __name__ == "__main__":
    test_name = Path(__file__).stem
    with nano_app() as app:

        app.assert_screen(Screen.Home)

        expression="0502000000f702000000f202000000ed02000000e802000000e302000000de02000000d902000000d402000000cf02000000ca02000000c502000000c002000000bb02000000b602000000b102000000ac02000000a702000000a2020000009d02000000980200000093020000008e02000000890200000084020000007f020000007a02000000750200000070020000006b02000000660200000061020000005c02000000570200000052020000004d02000000480200000043020000003e02000000390200000034020000002f020000002a02000000250200000020020000001b02000000160200000011020000000c02000000070200000002002a"

        if app.backend.firmware.device == "nanos":
            app.setup_blind_signing()

            def send(result_queue: Queue) -> None:
                res = app.backend.sign(DEFAULT_ACCOUNT, expression, with_hash=True)
                result_queue.put(res)
            def assert_screen_i(i):
                app.assert_screen(f"{str(i).zfill(5)}", path=(Path(test_name) / "clear"))

            result_queue: Queue = Queue()
            send_process = Process(target=send, args=(result_queue,))
            send_process.start()

            app.backend.wait_for_text_not_on_screen("Application")

            for i in range(4):
                assert_screen_i(i)
                app.backend.right_click()

            # 'Switch to blindsigning' screen
            assert_screen_i(i+1)

            def blind_navigate() -> None:
                app.navigate_until_text("Accept", Path(test_name) / "blind")
            navigate_process = Process(target=blind_navigate)
            navigate_process.start()

            app.backend.both_click()

            navigate_process.join()
            assert navigate_process.exitcode == 0, "Should have terminate successfully"

            send_process.join()
            assert send_process.exitcode == 0, "Should have terminate successfully"

            data = result_queue.get()
        else:
            data = app.blind_sign(DEFAULT_ACCOUNT,
                                  expression,
                                  with_hash=True,
                                  path=test_name)

        app.check_signature_with_hash(
            account=DEFAULT_ACCOUNT,
            message=expression,
            hash="93070b00990e4cf29c31f6497307bea0ad86a9d0dc08dba8b607e8dc0e23652f",
            data=data)

        app.quit()
