#!/usr/bin/env python3
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

from utils import *

if __name__ == "__main__":
    app = stax_app(__file__)

    app.assert_screen(SCREEN_HOME_DEFAULT, True)

    app.welcome.info()
    app.assert_screen("settings_blindsigning_off")


    app.settings_toggle_blindsigning()
    app.assert_screen("settings_blindsigning_on")

    app.settings_toggle_expert_mode()
    app.assert_screen("settings_blindsigning_expert_on")

    app.settings_toggle_blindsigning()
    app.assert_screen("settings_expert_on")

    app.settings_toggle_expert_mode()
    app.assert_screen("settings_blindsigning_off")


    app.info.next()
    app.assert_screen(SCREEN_INFO_PAGE, True)

    app.info.multi_page_exit()
    app.assert_screen(SCREEN_HOME_DEFAULT, True)

    app.quit()
