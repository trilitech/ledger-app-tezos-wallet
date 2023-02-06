all: app_nanos.tgz app_nanosp.tgz app_nanox.tgz
debug: app_nanos_dbg.tgz app_nanosp_dbg.tgz app_nanox_dbg.tgz

.PHONY: clean all debug integration_tests unit_tests

app/src/parser/generated_patterns.h: pattern_registry/*.ml* pattern_registry/Makefile pattern_registry/dune pattern_registry/*.csv.in pattern_registry/*.sh
	docker run --rm -i -v "$(realpath .):/app" ledger-app-tezos-ocaml:latest make -C /app/pattern_registry

app_%.tgz: app/src/parser/generated_patterns.h app/src/*.[ch] app/src/parser/*.[ch] app/Makefile
	docker run --rm -i -v "$(realpath .):/app" ledger-app-builder:latest bash -c \
          'BOLOS_SDK=$$$(shell echo $(patsubst app_%.tgz,%,$@) | tr '[:lower:]' '[:upper:]')_SDK make -C app'
	docker run --rm -i -v "$(realpath .):/app" ledger-app-builder:latest bash -c "cd app/bin/ && tar cz ." > $@

app_%_dbg.tgz: app/src/parser/generated_patterns.h app/src/*.[ch] app/src/parser/*.[ch] app/Makefile
	docker run --rm -i -v "$(realpath .):/app" ledger-app-builder:latest bash -c \
          'BOLOS_SDK=$$$(shell echo $(patsubst app_%_dbg.tgz,%,$@) | tr '[:lower:]' '[:upper:]')_SDK make -C app DEBUG=true'
	docker run --rm -i -v "$(realpath .):/app" ledger-app-builder:latest bash -c "cd app/bin/ && tar cz ." > $@

clean:
	rm -rf bin app_*.tgz
	docker run --rm -i -v "$(realpath .):/app" ledger-app-builder:latest make -C app mrproper
	docker run --rm -i -v "$(realpath .):/app" ledger-app-tezos-ocaml:latest bash -c \
	  "make -C /app/pattern_registry clean && make -C /app/tests/generate clean && rm -rf _build"

unit_tests: test/samples/micheline.hex tests/unit/*.ml* tests/unit/*.[ch] tests/unit/dune tests/unit/Makefile
	docker run --rm -i -v "$(realpath .):/app" ledger-app-tezos-ocaml:latest make -C /app/tests/unit

integration_tests: app_nanos_dbg.tgz test/samples/micheline.hex tests/integration/*.sh tests/integration/nanos/*.sh
	./tests/integration/run_test_docker.sh nanos app_nanos_dbg.tgz tests/integration/nanos
	./tests/integration/run_test_docker.sh nanos app_nanos_dbg.tgz tests/samples

test/samples/micheline.hex: tests/generate/*.ml* tests/generate/dune tests/generate/Makefile
	docker run --rm -i -v "$(realpath .):/app" ledger-app-tezos-ocaml:latest make -C /app/tests/generate
