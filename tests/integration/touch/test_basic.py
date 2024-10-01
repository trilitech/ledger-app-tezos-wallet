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

from utils import tezos_app, BlindsigningStatus

if __name__ == "__main__":
    app = tezos_app(__file__)

    app.assert_home()

    app.set_expert_mode(initial_status=False)
    app.set_expert_mode(initial_status=True)
    app.set_blindsigning_status(BlindsigningStatus.ON)
    app.set_blindsigning_status(BlindsigningStatus.OFF)

    app.welcome.settings()
    app.settings.next()
    app.settings.next()
    app.assert_info()

    app.settings.multi_page_exit()
    app.assert_home()

    app.quit()
