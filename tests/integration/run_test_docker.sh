#!/bin/bash -e

# Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

docker run --rm -i -v "$(realpath .):/app"		\
    --entrypoint=/bin/sh speculos			\
    -c "cd /app && apt-get update && apt-get -y install curl jq && SPECULOS=/speculos/speculos.py ./tests/integration/run_test_local.sh $*"
