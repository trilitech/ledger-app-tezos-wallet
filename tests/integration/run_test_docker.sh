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
    echo "Starting speculos in a Docker container on port $PORT..."
    volume=$(docker volume create speculos_$PORT)
    docker run --rm -i -v "$volume":/app			\
	--entrypoint "/bin/bash" speculos -c			\
	"mkdir -p /app/bin/$target/; tar xz -C /app/bin/$target/" < "$tgz"

    container=$(docker run --name speculos_$PORT --rm -i -d	\
			-v "speculos_$PORT":/app		\
			--publish $PORT:$PORT			\
			--network bridge			\
			speculos --display headless		\
			    --api-port $PORT --seed "$seed"	\
			    -m $target /app/bin/$target/app.elf)
    docker logs -f $container > $SPECULOG 2>&1 &
    attempts curl -s $SPECULOS_URL/events > /dev/null 2>&1
    attempts [ "$(curl -s $SPECULOS_URL/events 2> /dev/null)" != "{}" ]
}

function kill_speculos_runner {
    echo "Stopping speculos on port $PORT..."

    exited || docker container rm -f speculos_$PORT
    attempts exited

    docker volume rm "speculos_$PORT" > /dev/null || true
}

function exited {
    if docker container inspect speculos_$PORT > /dev/null 2>&1 ; then
        return 1
    else
        return 0
    fi
}

main "$@"
