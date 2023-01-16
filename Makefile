# Check for Docker
ifneq ($(shell docker --version > /dev/null && echo works), works)
  $(error "Docker is required, see README")
endif

# A named volume for the CI Docker-in-Docker, the current working dir on the dev's machine
BUILD_VOLUME ?= $(realpath .)

# Check for app building image
ifneq ($(shell docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-builder:latest echo works), works)
  $(error "Docker image `ledger-app-builder` is required, see README")
endif

# Check for app building image
ifneq ($(shell docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-tezos-ocaml:latest echo works), works)
  $(error "Docker image `ledger-app-tezos-ocaml` is required, see README")
endif

all: app_s app_sp app_x

.PHONY: clean app built-in-patterns

app_s: built-in-patterns
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-builder:latest bash -c 'BOLOS_SDK=$$NANOS_SDK make -C app'
	mkdir -p bin/s
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-builder:latest bash -c "cd app/bin/ && tar c ." | tar x -C bin/s

app_sp: built-in-patterns
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-builder:latest bash -c 'BOLOS_SDK=$$NANOSP_SDK make -C app'
	mkdir -p bin/sp
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-builder:latest bash -c "cd app/bin/ && tar c ." | tar x -C bin/sp

app_x: built-in-patterns
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-builder:latest bash -c 'BOLOS_SDK=$$NANOX_SDK make -C app'
	mkdir -p bin/x
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-builder:latest bash -c "cd app/bin/ && tar c ." | tar x -C bin/x

clean:
	rm -rf bin
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-builder:latest make -C app mrproper
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-tezos-ocaml:latest make -C /app/pattern_registry clean
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-tezos-ocaml:latest make -C /app/tests clean
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-tezos-ocaml:latest rm -rf _build

test:
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-tezos-ocaml:latest make -C /app/tests

built-in-patterns:
	docker run --rm -i -v "${BUILD_VOLUME}:/app" ledger-app-tezos-ocaml:latest make -C /app/pattern_registry

