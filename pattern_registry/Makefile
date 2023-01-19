CONTRACT_TYPES=$(patsubst %_patterns.csv.in,%,$(wildcard *_patterns.csv.in))

../app/src/parser/generated_patterns.h: $(addsuffix _patterns.csv,$(CONTRACT_TYPES)) contracts.csv
	./generate_c.sh $(addsuffix _patterns.csv,$(CONTRACT_TYPES)) > $@

%_patterns.csv: %_patterns.csv.in
	./preprocess_patterns.sh $< > $@

contracts.csv: contracts.csv.in
	./preprocess_contracts.sh $< > $@

.PHONT: clean

clean:
	rm -f *.csv
	rm -f ../app/src/parser/generated_patterns.h
