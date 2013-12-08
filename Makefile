CFLAGS= -g -Wall -Wextra -pedantic -O2
LDLIBS=
CC=c99

all: src/cs

src/cs: src/*.o

.PHONY: clean
clean:
	rm -f src/cs src/*.o