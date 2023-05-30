#
# Makefile
#

all: app_nanos.tgz app_nanosp.tgz app_nanox.tgz
debug: app_nanos_dbg.tgz app_nanosp_dbg.tgz app_nanox_dbg.tgz

.PHONY: clean all debug integration_tests unit_tests \
	integration_tests_basic integration_tests_basic_%

DOCKER			= docker
DOCKER_RUN		= $(DOCKER) run --rm -i -v "$(realpath .):/app"
DOCKER_RUN_APP_BUILDER	= $(DOCKER_RUN) ledger-app-builder:latest
DOCKER_RUN_APP_OCAML	= $(DOCKER_RUN) ledger-app-tezos-ocaml:latest

GENERATED_PATTERNS	= app/src/parser/generated_patterns.h

#
# Fetch the docker images:

LEDGERHQ=ghcr.io/ledgerhq

docker_speculos:
	$(DOCKER) pull $(LEDGERHQ)/speculos
	$(DOCKER) image tag $(LEDGERHQ)/speculos speculos

docker_ledger_app_builder:
	$(DOCKER) pull $(LEDGERHQ)/ledger-app-builder/ledger-app-builder:latest
	$(DOCKER) image tag $(LEDGERHQ)/ledger-app-builder/ledger-app-builder \
			ledger-app-builder

docker_ledger_app_ocaml:
	$(DOCKER) build -t ledger-app-tezos-ocaml \
			-f docker/Dockerfile.ocaml docker

docker_ledger_app_integration_tests:
	$(DOCKER) build -t ledger-app-tezos-integration-tests \
			-f docker/Dockerfile.integration-tests docker

docker_images:	docker_speculos			\
		docker_ledger_app_builder	\
		docker_ledger_app_ocaml		\
		docker_ledger_app_integration_tests

$(GENERATED_PATTERNS):	pattern_registry/*.ml*		\
			pattern_registry/Makefile	\
			pattern_registry/dune		\
			pattern_registry/*.csv.in	\
			pattern_registry/*.sh
	$(DOCKER_RUN_APP_OCAML) make -C /app/pattern_registry

app_%.tgz:	$(GENERATED_PATTERNS)	\
		app/src/*.[ch]		\
		app/src/parser/*.[ch]	\
		app/Makefile
	$(DOCKER_RUN_APP_BUILDER) bash -c \
          'BOLOS_SDK=$$$(shell echo $(patsubst app_%.tgz,%,$@) | tr '[:lower:]' '[:upper:]')_SDK make -C app'
	$(DOCKER_RUN_APP_BUILDER) bash -c "cd app/bin/ && tar cz ." > $@

app_%_dbg.tgz: $(GENERATED_PATTERNS) app/src/*.[ch] app/src/parser/*.[ch] app/Makefile
	$(DOCKER_RUN_APP_BUILDER) bash -c \
          'BOLOS_SDK=$$$(shell echo $(patsubst app_%_dbg.tgz,%,$@) | tr '[:lower:]' '[:upper:]')_SDK make -C app DEBUG=true'
	$(DOCKER_RUN_APP_BUILDER) bash -c "cd app/bin/ && tar cz ." > $@

clean:
	rm -rf bin app_*.tgz
	$(DOCKER_RUN_APP_BUILDER) make -C app mrproper
	$(DOCKER_RUN_APP_OCAML) bash -c \
	  "make -C /app/pattern_registry clean && make -C /app/tests/generate clean && cd /app && rm -rf _build"

unit_tests:	test/samples/micheline/samples.hex	\
		test/samples/operations/samples.hex	\
		tests/unit/*.ml*			\
		tests/unit/*.[ch]			\
		tests/unit/dune				\
		tests/unit/Makefile
	$(DOCKER_RUN_APP_OCAML) make -C /app/tests/unit

RUN_TEST_DOCKER = ./tests/integration/run_test_docker.sh

integration_tests_basic_%:	app_%_dbg.tgz			\
				tests/integration/*.sh		\
				tests/integration/%/*.sh
	$(RUN_TEST_DOCKER) $* app_$*_dbg.tgz			\
				tests/integration/$*		\

integration_tests_basic:	integration_tests_basic_nanos	\
				integration_tests_basic_nanosp	\
				integration_tests_basic_nanox

integration_tests_%:	integration_tests_basic_%		\
			test/samples/operations/samples.hex	\
			test/samples/micheline/samples.hex	\
			tests/integration/*.sh
	$(RUN_TEST_DOCKER) $* app_$*_dbg.tgz		\
				tests/samples/micheline/$*

integration_tests:	integration_tests_basic		\
			test/samples/operations/samples.hex	\
			test/samples/micheline/samples.hex	\
			tests/integration/*.sh
	$(RUN_TEST_DOCKER) nanos app_nanos_dbg.tgz		\
				tests/samples/operations	\
				tests/samples/micheline

test/samples/micheline/samples.hex:	tests/generate/*.ml*	\
					tests/generate/dune	\
					tests/generate/Makefile
	$(DOCKER_RUN_APP_OCAML) make -C /app/tests/generate	\
	    ../samples/micheline/samples.hex

test/samples/operations/samples.hex:	tests/generate/*.ml*	\
					tests/generate/dune	\
					tests/generate/Makefile
	$(DOCKER_RUN_APP_OCAML) make -C /app/tests/generate	\
	    ../samples/operations/samples.hex

load_%: app_%.tgz
	ledgerctl delete "Tezos Wallet"
	DIR=`mktemp -d` ; tar xf $< -C $$DIR && cd $$DIR && ledgerctl install app.toml ; rm -rf $$DIR

#
# Dash vs under aliases:

docker-images: docker_images
integration-tests: integration_tests
unit-tests: unit_tests
