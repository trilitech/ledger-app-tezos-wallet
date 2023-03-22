#!/bin/bash -e

. "`dirname $0`/test_runtime.sh"

function start_speculos_runner {
    echo "Starting speculos in a Docker container..."
    volume=`docker volume create`
    docker run --rm -i -v "$volume":/app			\
	--entrypoint "/bin/bash" speculos -c			\
	"mkdir -p /app/bin/$target/; tar xz -C /app/bin/$target/" < "$tgz"
    container=$(docker run --name speculos_runner --rm -i -d	\
			-v "$volume":/app			\
			--publish 5000:5000			\
			--network host				\
			speculos --display headless		\
			    --api-port 5000 --seed "$seed"	\
			    -m $target /app/bin/$target/app.elf)
    docker logs -f $container > $vars_dir/speculog 2>&1 &
    attempts curl -s localhost:5000/events 2> /dev/null >&2
    attempts [ "$(curl -s localhost:5000/events 2> /dev/null)" != "{}" ]
}

function kill_speculos_runner {
    echo "Stopping speculos..."

    exited || docker container rm -f speculos_runner
    attempts exited
    docker volume rm "$volume" > /dev/null 2>&1 || true
}

function exited {
    if docker container inspect speculos_runner > /dev/null 2>&1 ; then
        return 1
    else
        return 0
    fi
}

main "$@"
