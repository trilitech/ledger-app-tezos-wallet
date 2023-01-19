#!/bin/bash -e

if [ "$#" -ne 1 ] ; then
    echo "Expected a single argument" >&2
    exit 1
fi

target="$1"

if [ "$target" != "nanos" ] && [ "$target" != "nanosp" ] && [ "$target" != "nanox" ] ; then
    echo "Expected nanos, nanosp or nanox as argument" >&2
    exit 1
fi

seed="zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra zebra"

function start_speculos_runner {
    echo -n "Starting speculos in a Docker container..."
    volume=`docker volume create`
    docker run --rm -i -v "$volume":/app --entrypoint "/bin/bash" speculos -c "mkdir -p /app/bin/$target/ ; tar xz -C /app/bin/$target/" < "../../app_$target.tgz"
    docker run --name speculos_runner --rm -i -v "$volume":/app --publish 5000:5000 --network host --detach speculos --display headless --api-port 5000 --seed "$seed" -m $target /app/bin/$target/app.elf > /dev/null 2>&1

    while ! curl -s localhost:5000/events 2> /dev/null >&2 ; do sleep 0.1 ; echo -n "." ; done
    while [ "`curl -s localhost:5000/events 2> /dev/null`" == "{}" ] ; do sleep 0.1 ; echo -n "." ; done
    echo
}

function kill_speculos_runner {
    echo "Stopping speculos..."
    if docker container ls | grep speculos_runner > /dev/null 2>&1 ; then
        docker container kill speculos_runner > /dev/null 2>&1
    fi
    docker volume rm "$volume" > /dev/null 2>&1 || true
}

function expect_exited {
    echo " - expect_exited"
    attempts=20
    while [ $attempts -gt 0 ] && docker container ls | grep speculos_runner > /dev/null 2>&1 ; do
        sleep 0.5
    done
    if docker container ls | grep speculos_runner > /dev/null 2>&1 ; then
        echo "FAILURE(expect_exited)" >&2
        exit 1
    fi
}

function cleanup {
    echo "Failure."
    kill_speculos_runner
    rm -rf $vars_dir
}

. "$(pwd)/test_runtime.sh"

for i in $target/test_*.sh ; do
    vars_dir=`mktemp -d`
    start_speculos_runner
    trap cleanup EXIT
    echo "Running test $i"
    . "$(pwd)/$i"
    echo "Success."
    kill_speculos_runner
    rm -rf $vars_dir
    trap - EXIT
done
