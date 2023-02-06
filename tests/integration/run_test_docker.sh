#!/bin/bash -e

function start_speculos_runner {
    echo -n "Starting speculos in a Docker container..."
    volume=`docker volume create`
    docker run --rm -i -v "$volume":/app --entrypoint "/bin/bash" \
      speculos -c "mkdir -p /app/bin/$target/ ; tar xz -C /app/bin/$target/" < "$tgz"
    container=$(docker run --name speculos_runner --rm -i -v "$volume":/app --publish 5000:5000 --network host --detach \
                  speculos --display headless --api-port 5000 --seed "$seed" -m $target /app/bin/$target/app.elf 2>/dev/null)
    docker logs -f $container > $vars_dir/speculog 2>&1 &
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

function exited {
    if docker container ls | grep speculos_runner > /dev/null 2>&1 ; then
        return 1
    else
        return 0
    fi
}

. "`dirname $0`/test_runtime.sh"

main "$@"
