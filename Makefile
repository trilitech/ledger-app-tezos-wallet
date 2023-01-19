# A named volume for the CI Docker-in-Docker, the current working dir on the dev's machine
BUILD_VOLUME ?= $(realpath .)

all: build dist test

.PHONY: clean app build dist built-in-patterns test integration_tests integration_tests_s integration_tests_sp integration_tests_x unit_tests app_s app_sp app_x

# Check for Docker and required images
#ifneq ($(shell docker --version > /dev/null && echo works), works)
#  $(error "Docker is required, see README")
#endif
#ifneq ($(shell docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-builder:latest echo works), works)
#  $(error "Docker image `ledger-app-builder` is required, see README")
#endif
#ifneq ($(shell docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-tezos-ocaml:latest echo works), works)
#  $(error "Docker image `ledger-app-tezos-ocaml` is required, see README")
#endif

dist: build
	tar czf app_nanos.tgz -C bin/nanos .
	tar czf app_nanosp.tgz -C bin/nanosp .
	tar czf app_nanox.tgz -C bin/nanox .

build: app_s app_sp app_x

app_s: built-in-patterns
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-builder:latest bash -c 'BOLOS_SDK=$$NANOS_SDK make -C app'
	mkdir -p bin/nanos
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-builder:latest bash -c "cd app/bin/ && tar c ." | tar x -C bin/nanos
	tar czf app_nanos.tgz -C bin/nanos .

app_sp: built-in-patterns
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-builder:latest bash -c 'BOLOS_SDK=$$NANOSP_SDK make -C app'
	mkdir -p bin/nanosp
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-builder:latest bash -c "cd app/bin/ && tar c ." | tar x -C bin/nanosp
	tar czf app_nanosp.tgz -C bin/nanosp .

app_x: built-in-patterns
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-builder:latest bash -c 'BOLOS_SDK=$$NANOX_SDK make -C app'
	mkdir -p bin/nanox
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-builder:latest bash -c "cd app/bin/ && tar c ." | tar x -C bin/nanox
	tar czf app_nanox.tgz -C bin/nanox .

clean:
	rm -rf bin app_*.tgz
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-builder:latest make -C app mrproper
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-tezos-ocaml:latest make -C /app/pattern_registry clean
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-tezos-ocaml:latest make -C /app/tests/unit clean
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-tezos-ocaml:latest rm -rf _build

built-in-patterns:
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-tezos-ocaml:latest make -C /app/pattern_registry

test: unit_tests integration_tests

integration_tests: integration_tests_s # integration_tests_sp integration_tests_x

unit_tests:
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-tezos-ocaml:latest make -C /app/tests/unit

integration_tests_s: dist
	cd tests/integration && ./run_test_docker.sh nanos

integration_tests_sp: dist
	cd tests/integration && ./run_test_docker.sh nanosp

integration_tests_x: dist
	cd tests/integration && ./run_test_docker.sh nanox
