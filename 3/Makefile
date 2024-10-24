CC=gcc
FLAGS=-Wall -O2
LIBS=-lllhttp -lpthread
TARGET=proxy
OBJECTS=main.o log.o hash.o hashmap.o sieve.o http.o net.o stream.o buffer.o proxy.o

all: main

main: $(OBJECTS)
	$(CC) $(FLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)
	rm *.o

$(OBJECTS): %.o: %.c 
	$(CC) $(FLAGS) -c $<


clean:
	rm $(TARGET)
