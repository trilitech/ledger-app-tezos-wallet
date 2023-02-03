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
    echo -n "Starting speculos..."
    app_dir=`mktemp -d`
    if [ " $DEBUG " != "  " ] ; then dbg_suffix="_dbg" ; fi
    tar xfz ../../app_$target$dbg_suffix.tgz -C $app_dir
    speculos.py --display headless --api-port 5000 --seed "$seed" -m $target $app_dir/app.elf > /dev/null 2>&1 &
    speculos_pid=$!
    while ! curl -s localhost:5000/events > /dev/null 2>&1 ; do sleep 0.1 ; echo -n "." ; done
    while [ "`curl -s localhost:5000/events 2> /dev/null`" == "{}" ] ; do sleep 0.1 ; echo -n "." ; done
    echo
}

function kill_speculos_runner {
    echo "Stopping speculos..."
    kill -9 $speculos_pid > /dev/null 2>&1 || true
    rm -rf $app_dir
}

function expect_exited {
    echo " - expect_exited"
    attempts=20
    while ps $speculos_pid > /dev/null 2>&1 ; do
        sleep 0.5
    done
    if ps $speculos_pid > /dev/null 2>&1 ; then
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

for i in $target/test_*.sh ../samples/*.sh ; do
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

