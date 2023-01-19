.PHONY: all test_micheline_c_parser

all: test_micheline_c_parser

test_micheline_c_parser: *.ml *.[ch] dune Makefile
	dune exec ./test_micheline_c_parser.exe ../samples/micheline_samples.hex
