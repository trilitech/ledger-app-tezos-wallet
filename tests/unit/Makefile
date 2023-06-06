.PHONY: all test_micheline_c_parser test_operations_c_parser

all: test_micheline_c_parser test_operations_c_parser

test_micheline_c_parser: *.ml *.[ch] dune Makefile
	dune exec ./test_c_parser.exe micheline ../samples/micheline/samples.hex

test_operations_c_parser: *.ml *.[ch] dune Makefile
	dune exec ./test_c_parser.exe operations ../samples/operations/nanos/samples.hex
	dune exec ./test_micheline_c_parser.exe ../samples/micheline/nanos/samples.hex
