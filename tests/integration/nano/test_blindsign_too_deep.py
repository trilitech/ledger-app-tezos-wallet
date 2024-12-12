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

"""Check blindsigning on too deep expression"""

from multiprocessing import Process, Queue
from pathlib import Path

from utils.app import Screen, ScreenText, TezosAppScreen, DEFAULT_ACCOUNT
from utils.message import MichelineExpr

def test_blindsign_too_deep(app: TezosAppScreen):
    """Check blindsigning on too deep expression"""
    test_name = Path(__file__).stem

    app.assert_screen(Screen.HOME)

    expression = MichelineExpr([[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[{'int':42}]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]])

    if app.backend.firmware.device == "nanos":
        def send(result_queue: Queue) -> None:
            res = app.backend.sign(DEFAULT_ACCOUNT, expression, with_hash=True)
            result_queue.put(res)
        def assert_screen_i(i):
            app.assert_screen(f"{str(i).zfill(5)}", path=Path(test_name) / "clear")

        result_queue: Queue = Queue()
        send_process = Process(target=send, args=(result_queue,))
        send_process.start()

        app.backend.wait_for_text_not_on_screen(ScreenText.HOME)

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
            app.navigate_until_text(ScreenText.SIGN_ACCEPT, Path(test_name) / "blind")
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
