.PHONY: all test_micheline_c_parser

all: test_micheline_c_parser

test_micheline_c_parser: micheline_samples.hex *.ml *.[ch] dune Makefile
	dune exec ./test_micheline_c_parser.exe $<

micheline_samples.hex: *.ml *.[ch] dune Makefile
	dune exec ./gen_micheline.exe $@ 10000

clean:
	rm -rf micheline_samples.hex
