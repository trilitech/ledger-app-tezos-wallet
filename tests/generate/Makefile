../samples/micheline_samples.hex: *.ml* dune Makefile
	dune build ./gen_micheline.exe
	mkdir -p ../samples
	../../_build/default/tests/generate/gen_micheline.exe 10000 > ../samples/micheline_samples.hex
	n=1 ; tail -300 ../samples/micheline_samples.hex | while read hex ; do ../../_build/default/tests/generate/gen_integration.exe $$hex > ../samples/test_$$n.sh ; n=$$((n+1)) ; done

clean:
	rm -rf ../samples
