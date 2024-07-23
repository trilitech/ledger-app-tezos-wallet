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

from multiprocessing import Process, Queue
from pathlib import Path

from utils.app import Screen, Screen_text, DEFAULT_ACCOUNT
from utils.message import Message

# Expression: {{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{42}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}

def test_blindsign_too_deep(app):
    test_name = Path(__file__).stem

    app.assert_screen(Screen.Home)

    expression = Message.from_bytes("0502000000f702000000f202000000ed02000000e802000000e302000000de02000000d902000000d402000000cf02000000ca02000000c502000000c002000000bb02000000b602000000b102000000ac02000000a702000000a2020000009d02000000980200000093020000008e02000000890200000084020000007f020000007a02000000750200000070020000006b02000000660200000061020000005c02000000570200000052020000004d02000000480200000043020000003e02000000390200000034020000002f020000002a02000000250200000020020000001b02000000160200000011020000000c02000000070200000002002a")

    if app.backend.firmware.device == "nanos":
        def send(result_queue: Queue) -> None:
            res = app.backend.sign(DEFAULT_ACCOUNT, expression, with_hash=True)
            result_queue.put(res)
        def assert_screen_i(i):
            app.assert_screen(f"{str(i).zfill(5)}", path=(Path(test_name) / "clear"))

        result_queue: Queue = Queue()
        send_process = Process(target=send, args=(result_queue,))
        send_process.start()

        app.backend.wait_for_text_not_on_screen(Screen_text.Home)

        for i in range(6):
            # 'Review operation'
            # 'Expression {{{...{{{'
            # 'Expression {{{...{{{'
            # 'The transaction cannot be trusted.'
            # 'Parsing error ERR_TOO_DEEP'
            # 'Learn More: bit.ly/ledger-tez'
            assert_screen_i(i)
            app.backend.right_click()

        # 'Accept risk' screen
        assert_screen_i(i+1)

        def blind_navigate() -> None:
            app.navigate_until_text(Screen_text.Sign_accept, Path(test_name) / "blind")
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

    app.checker.check_signature(
        account=DEFAULT_ACCOUNT,
        message=expression,
        with_hash=True,
        data=data)

    app.quit()
