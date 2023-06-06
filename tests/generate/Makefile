.PHONY: nanos nanosp
nanos:
nanosp:

../samples/micheline/%/samples.hex: % *.ml* dune Makefile
	mkdir -p ../samples/micheline/$<
	dune exec ./generate.exe micheline 100 $< ../samples/micheline/

../samples/operations/%/samples.hex: % *.ml* dune Makefile
	mkdir -p ../samples/operations/$<
	dune exec ./generate.exe operations 100 $< ../samples/operations/

clean:
	rm -rf ../samples
