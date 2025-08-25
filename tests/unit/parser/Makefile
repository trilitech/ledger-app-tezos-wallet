.PHONY: all test_micheline_c_parser test_operations_c_parser

TESTS_ROOT = ../..
ABS_DIR = unit/parser

all: test_micheline_c_parser test_operations_c_parser

test_micheline_c_parser: *.ml *.[ch] dune Makefile
	dune exec --root=$(TESTS_ROOT) \
		$(ABS_DIR)/test_c_parser.exe \
		micheline \
		$(TESTS_ROOT)/samples/micheline/nano/samples.hex

test_operations_c_parser: *.ml *.[ch] dune Makefile
	dune exec --root=$(TESTS_ROOT) \
		$(ABS_DIR)/test_c_parser.exe \
		operations \
		$(TESTS_ROOT)/samples/operations/nano/samples.hex
