CC=gcc
FLAGS=-Wall -O2
LIBS=./llhttp/build/libllhttp.a -lpthread
TARGET=proxy
INCLUDES=-I./llhttp/build/
OBJECTS=main.o log.o hash.o hashmap.o sieve.o http.o net.o stream.o buffer.o proxy.o

all: main

main: $(OBJECTS) libllhttp
	$(CC) $(FLAGS) -o $(TARGET) $(OBJECTS) $(LIBS) $(INCLUDES)
	rm *.o

$(OBJECTS): %.o: %.c 
	$(CC) $(FLAGS) -c $<

libllhttp:
	(cd llhttp && npm ci && make)


clean:
	rm $(TARGET)
	(cd llhttp && make clean)
