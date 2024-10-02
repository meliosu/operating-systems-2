CC=gcc
FLAGS=-Wall -g -O2

OBJECTS=log.o http.o hash.o slice.o hashmap.o sieve.o main.o
OUT_DIR=target

all: main

main: $(OBJECTS)
	$(CC) $(FLAGS) -o target/proxy $(OBJECTS)

main.o:
	$(CC) $(FLAGS) -c main.c

log.o:
	$(CC) $(FLAGS) -c log.c

http.o:
	$(CC) $(FLAGS) -c http.c

hash.o:
	$(CC) $(FLAGS) -c hash.c

slice.o:
	$(CC) $(FLAGS) -c slice.c

hashmap.o:
	$(CC) $(FLAGS) -c hashmap.c

sieve.o:
	$(CC) $(FLAGS) -c sieve.c


clean:
	rm target/* *.o

