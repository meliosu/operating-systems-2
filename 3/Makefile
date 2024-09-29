CC=gcc
FLAGS=-Wall -g

OBJECTS=log.o 
OUT_DIR=target

all: main

main: main.o $(OBJECTS)
	$(CC) $(FLAGS) -o target/proxy main.o $(OBJECTS)

log.o:
	$(CC) $(FLAGS) -c log.c


clean:
	rm target/* *.o

