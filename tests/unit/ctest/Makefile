CCFLAGS=-Wall -Wextra -Wconversion -Wredundant-decls -Wshadow -Wno-unused-parameter -O3

.PROXY: run clean remake all

all: test run

remake: clean all

%.c.o: %.c ctest.h
	$(CC) $(CCFLAGS) -c -o $@ $<

test: main.c.o ctest.h
	$(CC) $(LDFLAGS) \
	digestif/sha256.c \
	../../../app/src/parser/formatting.c \
	../../../app/src/parser/parser_state.c \
	../../../app/src/parser/num_parser.c \
	../../../app/src/parser/micheline_parser.c \
	../../../app/src/parser/operation_parser.c \
	-I../../../app/src/parser \
	tests_parser.c \
	main.c.o -o test

run: test
	./test

clean:
	rm -f test *.o
