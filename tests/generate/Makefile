.PHONY: nanos
nanos:

../samples/micheline/%/samples.hex: % *.ml* dune Makefile
	mkdir -p ../samples/micheline/$<
	dune exec ./generate.exe micheline 1000 $< ../samples/micheline/

../samples/operations/%/samples.hex: % *.ml* dune Makefile
	mkdir -p ../samples/operations/$<
	dune exec ./generate.exe operations 1000 $< ../samples/operations/

clean:
	rm -rf ../samples
