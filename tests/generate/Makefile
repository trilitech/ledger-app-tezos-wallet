../samples/micheline/samples.hex: *.ml* dune Makefile
	mkdir -p ../samples/micheline/nanos
	dune exec ./generate.exe micheline 1000 nanos ../samples/micheline/

../samples/operations/samples.hex: *.ml* dune Makefile
	mkdir -p ../samples/operations
	dune exec ./generate.exe operations 1000 nanos ../samples/operations/

clean:
	rm -rf ../samples
