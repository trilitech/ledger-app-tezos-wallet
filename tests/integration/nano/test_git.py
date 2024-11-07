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

"""Module to test git instruction."""

from utils.app import Screen, TezosAppScreen

def test_git(app: TezosAppScreen):
    """Test that the app commit is the same as the current git commit."""
    app.assert_screen(Screen.HOME)

    data = app.backend.git()

    app.checker.check_commit(data)

    app.quit()
