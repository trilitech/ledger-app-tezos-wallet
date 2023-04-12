# Tezos Ledger Wallet App Revamped

## Compile using `docker` containers for build dependencies

We compile and test using docker containers to obtain a reasonably
predictable environment despite some level of heterogeny in the build
setups of existing and potential developers.

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

NOTE: the tests are [currently] only available for the Nano S.

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
