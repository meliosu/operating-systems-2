CC=gcc
FLAGS=-Wall -g -O2

OBJECTS=log.o http.o hash.o
OUT_DIR=target

all: main

main: main.o $(OBJECTS)
	$(CC) $(FLAGS) -o target/proxy main.o $(OBJECTS)

log.o:
	$(CC) $(FLAGS) -c log.c

http.o:
	$(CC) $(FLAGS) -c http.c

hash.o:
	$(CC) $(FLAGS) -c hash.c


clean:
	rm target/* *.o

