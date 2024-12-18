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

"""Check main menu operation"""

from utils.app import Screen, TezosAppScreen

def test_basic(app: TezosAppScreen):
    """Check main menu operation"""
    app.assert_screen(Screen.HOME)
    app.backend.right_click()
    app.assert_screen(Screen.VERSION)
    app.backend.both_click()
    app.assert_screen(Screen.VERSION)
    app.backend.right_click()
    app.assert_screen(Screen.SETTINGS)
    app.backend.right_click()
    app.assert_screen(Screen.QUIT)
    app.backend.left_click()
    app.assert_screen(Screen.SETTINGS)
    app.backend.left_click()
    app.assert_screen(Screen.VERSION)
    app.backend.left_click()
    app.assert_screen(Screen.HOME)
    app.backend.both_click()
    app.assert_screen(Screen.HOME)
    app.backend.left_click()
    app.assert_screen(Screen.QUIT)
    app.backend.right_click()
    app.assert_screen(Screen.HOME)
    app.backend.right_click()
    app.assert_screen(Screen.VERSION)
    app.backend.right_click()
    app.assert_screen(Screen.SETTINGS)
    app.backend.right_click()
    app.assert_screen(Screen.QUIT)
    app.backend.right_click()
    app.assert_screen(Screen.HOME)
    app.backend.right_click()
    app.assert_screen(Screen.VERSION)

    # Check Settings menu operation
    app.backend.right_click()
    app.assert_screen(Screen.SETTINGS)
    app.backend.both_click()
    app.assert_screen(Screen.SETTINGS_EXPERT_MODE_DISABLED)
    app.backend.both_click()
    app.assert_screen(Screen.SETTINGS_EXPERT_MODE_ENABLED)
    app.backend.right_click()
    app.assert_screen(Screen.SETTINGS_BLINDSIGN_OFF)
    app.backend.both_click()
    app.assert_screen(Screen.SETTINGS_BLINDSIGN_ON)
    app.backend.both_click()
    app.assert_screen(Screen.SETTINGS_BLINDSIGN_OFF)
    app.backend.right_click()
    app.assert_screen(Screen.SETTINGS_BACK)
    app.backend.left_click()
    app.assert_screen(Screen.SETTINGS_BLINDSIGN_OFF)
    app.backend.left_click()
    app.assert_screen(Screen.SETTINGS_EXPERT_MODE_ENABLED)
    app.backend.left_click()
    app.assert_screen(Screen.SETTINGS_BACK)
    app.backend.right_click()
    app.assert_screen(Screen.SETTINGS_EXPERT_MODE_ENABLED)
    app.backend.left_click()
    app.assert_screen(Screen.SETTINGS_BACK)
    app.backend.both_click()
    app.assert_screen(Screen.HOME)
    app.backend.left_click()
    app._quit()
