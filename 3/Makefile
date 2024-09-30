CC=gcc
FLAGS=-Wall -g

OBJECTS=log.o http.o
OUT_DIR=target

all: main

main: main.o $(OBJECTS)
	$(CC) $(FLAGS) -o target/proxy main.o $(OBJECTS)

log.o:
	$(CC) $(FLAGS) -c log.c

http.o:
	$(CC) $(FLAGS) -c http.c


clean:
	rm target/* *.o

