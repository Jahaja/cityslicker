CFLAGS=-g -Wall -Wextra -pedantic -O2
LDFLAGS=
CC=c99

all: cs

cs: src/cs.o src/net.o src/geonames.o src/util.o 
	$(CC) -o $@ $^

clean:
	rm -f cs src/*.o

.PHONY: all distclean clean msgpack