CC=gcc
FLAGS=-Wall -g

OBJECTS=main.o log.o hash.o
TARGET=proxy

all: main

main: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS)

main.o:
	$(CC) $(FLAGS) -c main.c

log.o:
	$(CC) $(FLAGS) -c log.c

hash.o:
	$(CC) $(FLAGS) -c hash.c


clean:
	rm *.o $(TARGET)
