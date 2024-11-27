#!/usr/bin/env python3
# Copyright 2024 Functori <contact@functori.com>
# Copyright 2024 Trilitech <contact@trili.tech>

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

# http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Gathering of tests related to app version."""

import git

from utils.app import TezosAppScreen
from utils.backend import Version


def test_version(app: TezosAppScreen):
    """Test that the app version is the same as the current version."""
    current_version = Version(Version.AppKind.WALLET, 3, 0, 6)

    data = app.backend.version()

    app_version = Version.from_bytes(data)

    assert current_version == app_version, \
        f"Expected {current_version} but got {app_version}"


def test_git(app: TezosAppScreen):
    """Test that the app commit is the same as the current git commit."""
    git_repo = git.Repo(search_parent_directories=True)
    git_describe = git_repo.git.describe(
        tags=True,
        abbrev=8,
        always=True,
        long=True,
        dirty=True
    )
    current_commit = git_describe.replace('-dirty', '*')

    data = app.backend.git()

    assert data.endswith(b'\x00'), \
        f"Should end with by '\x00' but got {data.hex()}"

    app_commit = data[:-1].decode('utf-8')

    assert current_commit == app_commit, \
        f"Expected {current_commit} but got {app_commit}"
