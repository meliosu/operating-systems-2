CC=gcc
FLAGS=-Wall -O2

LIBS=-lllhttp -lpthread
OBJECTS=main.o log.o hash.o hashmap.o sieve.o http.o net.o stream.o buffer.o proxy.o
TARGET=proxy

all: main

main: $(OBJECTS)
	$(CC) $(FLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

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

buffer.o:
	$(CC) $(FLAGS) -c buffer.c

proxy.o:
	$(CC) $(FLAGS) -c proxy.c


clean:
	rm *.o $(TARGET)
