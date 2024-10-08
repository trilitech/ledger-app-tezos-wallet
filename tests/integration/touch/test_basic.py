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

from utils import tezos_app

if __name__ == "__main__":
    app = tezos_app(__file__)

    app.assert_home()
    app.remove_settings_and_info()

    app.welcome.settings()
    app.assert_settings()

    app.settings.toggle_blindsigning()
    app.assert_settings(blindsigning=True)

    app.settings.toggle_expert_mode()
    app.assert_settings(blindsigning=True, expert_mode=True)

    app.settings.toggle_blindsigning()
    app.assert_settings(expert_mode=True)

    app.settings.toggle_expert_mode()
    app.assert_settings()

    app.settings.next()
    app.assert_info()

    app.settings.exit()
    app.assert_home()

    app.quit()
