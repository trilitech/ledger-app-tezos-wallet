#!/usr/bin/env bash

# Copyright 2023 Functori <contact@functori.com>
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

set -x
set -e

usage() {
    echo "Usage: ./scripts/test_swap.sh {build_app_tezos|build_app_ethereum|build_app_exchange|run_tests|run_tests_all|update} {nanos|nanosp|nanox|stax|flex}"
}

_assert_tezos_repo() {
    set +x
    message="APP_TEZOS_REPO must refer to a clone of the https://github.com/trilitech/ledger-app-tezos-wallet repository"
    if [ ! -v APP_TEZOS_REPO ]; then
        echo "The variable \$APP_TEZOS_REPO is missing"
        echo $message
        usage
        exit 1
    fi
    if [ ! -d "$APP_TEZOS_REPO" ]; then
        echo $message
        usage
        exit 1
    fi
    set -x
}

_assert_eth_repo() {
    set +x
    message="APP_ETH_REPO must refer to a clone of the https://github.com/LedgerHQ/app-ethereum repository"
    if [ ! -v APP_ETH_REPO ]; then
        echo "The variable \$APP_ETH_REPO is missing"
        echo $message
        usage
        exit 1
    fi
    if [ ! -d "$APP_ETH_REPO" ]; then
        echo $message
        usage
        exit 1
    fi
    set -x
}

_assert_app_exchange_repo() {
    set +x
    message="APP_EXCHANGE_REPO must refer to a clone of the https://github.com/functori/app-exchange repository"
    if [ ! -v APP_EXCHANGE_REPO ]; then
        echo "The variable \$APP_EXCHANGE_REPO is missing"
        echo $message
        usage
        exit 1
    fi
    if [ ! -d "$APP_EXCHANGE_REPO" ]; then
        echo $message
        usage
        exit 1
    fi
    set -x
}

devices=("nanos" "nanosp" "nanox" "stax" "flex")

_assert_device() {
    set +x
    device=$1
    not_in=true
    for d in "${devices[@]}"; do
        if [ "$d" == "$device" ]; then
            not_in=false
            break
        fi
    done

    if $not_in; then
        echo "Wrong device: $device"
        usage
        exit 1
    fi
    set -x
}

_build_app() {
    _assert_device $1

    device=$1
    repo=$2
    params="$3"
    sdk=$(echo $device | tr "[:lower:]" "[:upper:]")_SDK

    (
        cd $repo

        docker run --rm -ti -v "$(realpath .):/app" --privileged ledger-app-builder:latest \
               bash -c "make clean && make -j $params BOLOS_SDK=\$$sdk"
    )
}

build_app_exchange() {
    _assert_app_exchange_repo

    _build_app $1 $APP_EXCHANGE_REPO "TESTING=1 TEST_PUBLIC_KEY=1 DEBUG=1"
}

_build_side_app() {
    _assert_app_exchange_repo

    device=$1
    name=$2
    repo=$3
    name_elf=$name"_"$(echo $device | tr 'sp' 's2')".elf"
    params="COIN=$name CHAIN=$name DEBUG=1"

    _build_app $device $repo $params

    cp -f $repo/bin/app.elf $APP_EXCHANGE_REPO/test/python/lib_binaries/$name_elf
}

build_app_ethereum() {
    _assert_eth_repo

    _build_side_app $1 "ethereum" $APP_ETH_REPO
}

build_app_tezos() {
    _assert_tezos_repo

    _build_side_app $1 "tezos" $APP_TEZOS_REPO/app
}

run_tests() {
    _assert_app_exchange_repo

    device=$1
    shift

    (
        cd $APP_EXCHANGE_REPO

        docker run --privileged  --entrypoint /bin/bash                   \
               --rm -v "$(realpath .):/app"                               \
               ledger-app-tezos-integration-tests -c                      \
               "cd /app &&                                                \
                pip install --upgrade pip -q &&                           \
                if [ \"\$device\" = \"nanos\" ]; then                     \
                    pip install -r test/python/requirements-nanos.txt -q; \
                else                                                      \
                    pip install -r test/python/requirements.txt -q;       \
                fi &&                                                     \
                pip install protobuf==3.20.3 && pytest test/python $*"
    )
}

run_tests_all() {
    _assert_device $1

    device=$1
    shift

    run_tests $device --device $device -k "tezos" $*
}

update() {
    device=$1

    build_app_tezos $device
    build_app_ethereum $device
    build_app_exchange $device
    run_tests_all $device --tb=short -v --golden_run
    run_tests_all $device --tb=short -v
}

case $1 in
    build_app_exchange)
        build_app_exchange $2
        ;;

    build_app_ethereum)
        build_app_ethereum $2
        ;;

    build_app_tezos)
        build_app_tezos $2
        ;;

    run_tests)
        shift
        run_tests $*
        ;;

    run_tests_all)
        shift
        run_tests_all $*
        ;;

    update)
        update $2
        ;;

    *)
        usage
        exit 1
        ;;
esac
