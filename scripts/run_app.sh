#!/usr/bin/env bash

# Copyright 2023 Nomadic Labs <contact@nomadic-labs.com>
# Copyright 2023 TriliTech <contact@trili.tech>
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

seed="zebra`for i in $(seq 1 23) ; do echo -n ' zebra' ; done`"

docker run --rm -it -v $(pwd)/app/bin:/speculos/apps \
       -v $(pwd)/tests/integration:/tests --network host \
       ledger-app-tezos-integration-tests --display=headless --vnc-port 41000 \
       --seed "$seed" -m $TARGET apps/app.elf
