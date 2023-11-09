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

if __name__ == "__main__":
    with nano_app() as app:

        app.assert_screen(Screen.Home)
        app.backend.right_click()
        app.assert_screen(Screen.Version)
        app.backend.both_click()
        app.assert_screen(Screen.Version)
        app.backend.right_click()
        app.assert_screen(Screen.Settings)
        app.backend.right_click()
        app.assert_screen(Screen.Quit)
        app.backend.left_click()
        app.assert_screen(Screen.Settings)
        app.backend.left_click()
        app.assert_screen(Screen.Version)
        app.backend.left_click()
        app.assert_screen(Screen.Home)
        app.backend.left_click()
        app.assert_screen(Screen.Home)
        app.backend.both_click()
        app.assert_screen(Screen.Home)
        app.backend.right_click()
        app.assert_screen(Screen.Version)
        app.backend.right_click()
        app.assert_screen(Screen.Settings)
        app.backend.right_click()
        app.assert_screen(Screen.Quit)
        app.backend.right_click()
        app.assert_screen(Screen.Quit)
        app._quit()
