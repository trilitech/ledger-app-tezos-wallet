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

set -e

. "`dirname $0`/app_vars.sh"

docker run --rm -it -v "$(realpath .):/app"              	\
       --env-file "`dirname $0`/err_codes.sh"           \
       -e VERSION_BYTES=$VERSION_BYTES                  \
       -e COMMIT_BYTES=$COMMIT_BYTES                    \
       -e VERSION_WALLET_TAG=$VERSION_WALLET_TAG        \
       -e APPVERSION_M=$APPVERSION_M                    \
       -e APPVERSION_N=$APPVERSION_N                    \
       -e APPVERSION_P=$APPVERSION_P                    \
       -e APPVERSION=$APPVERSION                        \
       -e VERSION_BYTES=$VERSION_BYTES                  \
    --entrypoint=/bin/bash ledger-app-tezos-integration-tests	   \
    -c " apt install -y libsodium-dev; \
		python3 -m venv tezos_test_env --system-site-package;      \
		source ./tezos_test_env/bin/activate;                      \
		python3 -m pip install --upgrade pip -q;    \
		if [ \"$1\" = \"nanos\" ]; then \
		    python3 -m pip install -r tests/requirements-nanos.txt -q; \
		else \
		    python3 -m pip install -r tests/requirements.txt -q; \
		fi; \
     ./tests/integration/run_test_local.sh -F -m $*"
