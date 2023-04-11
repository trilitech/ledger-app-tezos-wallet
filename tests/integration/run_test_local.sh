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

. "`dirname $0`/test_runtime.sh"

function start_speculos_runner {
    echo "Starting speculos..."
    app_dir=`mktemp -d`
    tar xfz "$tgz" -C $app_dir
    speculos.py --display headless --api-port 5000 --seed "$seed"	\
      -m $target $app_dir/app.elf > $vars_dir/speculog 2>&1 &
    speculos_pid=$!
    attempts curl -s localhost:5000/events > /dev/null 2>&1
    attempts [ "`curl -s localhost:5000/events 2> /dev/null`" != "{}" ]
}

function kill_speculos_runner {
    echo "Stopping speculos..."
    kill -9 $speculos_pid > /dev/null 2>&1 || true
    wait $speculos_pid 2> /dev/null || true
    rm -rf $app_dir
}


function exited {
    if ps $speculos_pid > /dev/null 2>&1 ; then
        return 1
    else
        return 0
    fi
}

main "$@"
