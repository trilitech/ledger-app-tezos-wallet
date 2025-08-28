.PHONY: nano stax
nano:

TESTS_ROOT = ..
ABS_DIR = generate

../samples/micheline/%/samples.hex: % *.ml* dune Makefile
	mkdir -p $(TESTS_ROOT)/samples/micheline/$<
	dune exec --root=$(TESTS_ROOT) \
		$(ABS_DIR)/generate.exe \
		micheline \
		1000 \
		$< \
		$(TESTS_ROOT)/samples/micheline/

../samples/operations/%/samples.hex: % *.ml* dune Makefile
	mkdir -p $(TESTS_ROOT)/samples/operations/$<
	dune exec --root=$(TESTS_ROOT) \
		$(ABS_DIR)/generate.exe \
		operations \
		1000 \
		$< \
		$(TESTS_ROOT)/samples/operations/

clean:
	rm -rf $(TESTS_ROOT)/samples
