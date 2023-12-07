CCFLAGS=-Wall -Wextra -Wconversion -Wredundant-decls -Wshadow -Wno-unused-parameter -O3

.PROXY: run clean remake all

all: test run

remake: clean all

%.c.o: %.c ctest.h
	$(CC) $(CCFLAGS) -c -o $@ $<

test: main.c.o ctest.h
	$(CC) $(LDFLAGS) main.c.o -o test

run: test
	./test

clean:
	rm -f test *.o
