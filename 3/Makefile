CC=gcc
FLAGS=-Wall -g

OBJECTS=main.o log.o hash.o hashmap.o
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

hashmap.o:
	$(CC) $(FLAGS) -c hashmap.c


clean:
	rm *.o $(TARGET)
