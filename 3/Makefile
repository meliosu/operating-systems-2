CC=gcc
FLAGS=-Wall -g

OBJECTS=main.o log.o hash.o hashmap.o sieve.o http.o net.o stream.c
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

sieve.o:
	$(CC) $(FLAGS) -c sieve.c

http.o:
	$(CC) $(FLAGS) -c http.c

net.o:
	$(CC) $(FLAGS) -c net.c

stream.o:
	$(CC) $(FLAGS) -c stream.c


clean:
	rm *.o $(TARGET)
