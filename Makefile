CC=gcc
CFLAGS=-I.

ghk: ghk.o
	gcc -o ghk ghk.o

clean:
	rm -f *.o ghk
