CC=gcc
FLAGS=-Wall -g -O2

all: 2.4 2.4-test

2.4-test:
	$(CC) $(FLAGS) -o 2.4-test 2.4-test.c -L. -llocks

2.4:
	$(CC) $(FLAGS) -o locks.o -c 2.4.c
	$(CC) -shared -o liblocks.so locks.o


clean:
	rm *.o *.so 2.4-test
