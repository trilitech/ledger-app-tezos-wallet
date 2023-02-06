#!/bin/bash -e

function start_speculos_runner {
    echo -n "Starting speculos..."
    app_dir=`mktemp -d`
    tar xfz "$tgz" -C $app_dir
    speculos.py --display headless --api-port 5000 --seed "$seed" -m $target $app_dir/app.elf > $vars_dir/speculog 2>&1 &
    speculos_pid=$!
    while ! curl -s localhost:5000/events > /dev/null 2>&1 ; do sleep 0.1 ; echo -n "." ; done
    while [ "`curl -s localhost:5000/events 2> /dev/null`" == "{}" ] ; do sleep 0.1 ; echo -n "." ; done
    echo
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

. "`dirname $0`/test_runtime.sh"

main "$@"
