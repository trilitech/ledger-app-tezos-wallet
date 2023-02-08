../samples/micheline_samples.hex: *.ml* dune Makefile
	mkdir -p ../samples
	dune exec ./generate.exe micheline 1000 nanos ../samples/

clean:
	rm -rf ../samples
