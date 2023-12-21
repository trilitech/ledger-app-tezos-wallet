# Tezos Ledger Wallet App

## Interfaces

Both `webusb` and `u2f` interfaces are supported. For example, `u2f` interface is used by the [Kukai](https://wallet.kukai.app/connect-ledger) web wallet.

## Compile using `docker` containers for build dependencies

We compile and test using docker containers to obtain a reasonably
predictable environment despite some level of heterogeny in the build
setups of existing and potential developers.

Please use bash shell for uniformity in build environment.

The docker images can be built using the provided Makefile:

```
:; make docker-images
```

This pulls down two images, `ghcr.io/ledgerhq/speculos` and
`ghcr.io/ledgerhq/ledger-app-builder/ledger-app-builder:latest`.  It
also builds an image via `docker/Dockerfile.ocaml`.

We do not make the builds of the software conditional on these docker
images as it can be time consuming to build them, approx 1 hour, and
`make(1)` does not automatically avoid rebuilds as they do not create
artefacts as files in the current directories.

## Make the applications

You're all set, just run `make` now, binaries for each `<model>` (S, S+
or X) should be available as `tgz` archives at the root.

```
:; make
```

Each target can build independently:

E.g.:

```
:; make app_nanos.tgz
```

To build the debugging targets, a similar setup is used:

```
:; make debug
```

Or for a single target:

```
:; make app_nanos_dbg.tgz
```

## Loading on real hardware

You need the `ledgetctl` tool, that can be installed with pip. At the
moment, only the unreleased version works, that you can install as
follows.

```
pip install git+https://github.com/LedgerHQ/ledgerctl
```

Depending on the hardware you have, call one of the two Makefile
targets `load_nanos` (for Nano S) or `load_nanosp` (for Nano S+).
Unfortunately, Nano X applications cannot be tested on real hardware.
Make sure you have the latest firmware on the device.

## Testing

There are two main kinds of testing that are provided: unit tests; and
integration tests.  The latter are much more time consuming, although
a framework is provided with has a lot of parallelism.  To run the
integration tests, you will need a box that can run a 32 concurrent
containers each running speculos.

The tests use the debugging targets and depend on the targets that
they use.

The commands are:

```
:; make unit-tests
```

and

```
:; make integration-tests
```

NOTE: the full integration tests are [currently] only available for
the Nano device. The basic tests can be run for all devices with:

```
:; make integration_tests_basic
```

> Tests can be run for individual devices by appending the device name: e.g. `integration_tests_basic_nanosp`.

Integration tests can be run directly from the command line rather than
via the Makefile:


```
:; ./tests/integration/run_test_docker.sh nanos app_nanos_dbg.tgz \
                                                tests/integration/nanos
```

There is also `run_test_local.sh` which should perform in a similar
fashion, but it doesn't run speculos in a docker container and so you
have to have the appropriate environment set up locally.

Both of these commands take the following arguments:

-T test_name
: only runs a test with this name, may be provided multiple times.

-l num
: stops after num tests.

-x
: executes the tests with shell tracing (-x)

Basic tests rely on gold-images, rather than OCR. They are stored under [nano/snapshots](./tests/integration/nano/snapshots) and [stax/snapshots](./tests/integration/stax/snapshots).

To generate/reset the snapshots, you can do so for individual tests.

### Nano

#### Preparation

First, start a container for running individual tests:

```sh
docker run --rm -it --entrypoint /bin/bash -v $(pwd):/app --network host \
  ledger-app-tezos-integration-tests

cd /app

git config --global --add safe.directory /app
```

Before running the test, build the nano app you want to test, for example the nanox app:

```sh
make app_nanox_dbg.tgz
```

#### Running

You can run an individual test from the test container. You should see the app progress on the vnc viewer.

```sh
./tests/integration/nano/<test_name>.py \
   --device $DEVICE \
   --port $PORT \
   --vnc-port 41000 \
   --app app/bin/app.elf
```

#### Setting goldimages

You can reset/set goldimages using the `--golden-run` option:

You will be requested to press enter to take snapshots in term.
**NB** make sure that the screen has updated to the screen you want to snapshot each time. It's also a good idea to
re-run the test normally afterwards, to ensure the snapshots have been set correctly.

### STAX

#### Preparation

First, start a container for running individual tests:

```sh
docker run --rm -it --entrypoint /bin/bash -v $(pwd):/app --network host \
  ledger-app-tezos-integration-tests

cd /app/tests/integration/stax
export PORT=5000

git config --global --add safe.directory /app
. ../app_vars.sh
```

Before running the test, start the app in a separate container (as each test will quit the app):

```sh
make app_stax_dbg.tgz

TARGET=stax ./scripts/run_app.sh
```

You can view/interact with the app using a vnc client on port `41000`.

#### Running

You can run an individual test from the test container. You should see the app progress on the vnc viewer.

```sh
./<test_name>.py
```

#### Setting goldimages

You can reset/set goldimages using the following:

```sh
GOLDEN=1 ./<test_name>.py
```

If you are resetting goldimages for multiple tests, you can also use `export NOQUIT=1` to keep the app
open at the end of a test.


You will be requested to press enter to take each snapshot in term.
**NB** make sure that the screen has updated to the screen you want to snapshot each time. It's also a good idea to
re-run the test normally afterwards, to ensure the snapshots have been set correctly.

## Swap test

### Requirement

Our swap tests are located in the https://github.com/functori/app-exchange repository.
You must have a clone of this repository. In the commands below, it must be referenced in the `APP_EXCHANGE_REPO` variable.

In some of the commands below, other variables will be required:
- `APP_TEZOS_REPO`: your clone of the https://github.com/trilitech/ledger-app-tezos-wallet repository
- `APP_ETH_REPO`: your clone of the https://github.com/LedgerHQ/app-ethereum repository
The commands below will consider the version of the current branch of each repository.

### Preparation

Make sure that the Tezos, Ethereum, and Exchange apps are built and set up correctly in the app-exchange repository.
If not, run :
```sh
./scripts/test_swap.sh build_app_exchange $DEVICE
./scripts/test_swap.sh build_app_ethereum $DEVICE
./scripts/test_swap.sh build_app_tezos    $DEVICE
```

### Running

You can run all  test with :
```sh
./scripts/test_swap.sh run_tests_all $DEVICE
```

And you can run an individual test with :
```sh
./scripts/test_swap.sh run_tests --device $DEVICE -v -k "your_test_name"
```

### Update

You can run :
```sh
./scripts/test_swap.sh update $DEVICE
```
to perform all snapshot update steps based on your current Tezos repository.
