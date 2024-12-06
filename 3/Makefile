CC=gcc
FLAGS=-Wall -O2
LIBS=./llhttp/build/libllhttp.a -lpthread
TARGET=proxy
INCLUDES=-I./llhttp/build/
OBJECTS=main.o log.o hash.o hashmap.o sieve.o http.o net.o stream.o buffer.o proxy.o
TESTS=tests/http

all: main

main: $(OBJECTS) libllhttp
	$(CC) $(FLAGS) -o $(TARGET) $(OBJECTS) $(LIBS) $(INCLUDES)

$(OBJECTS): %.o: %.c 
	$(CC) $(FLAGS) -c $<

libllhttp: ./llhttp/build/libllhttp.a

./llhttp/build/libllhttp.a:
	(cd llhttp && npm ci && make)

tests/http: tests/http.c http.o libllhttp
	$(CC) $(FLAGS) -o $@ $< http.o $(LIBS) $(INCLUDES)

test: $(TESTS)
	./tests/http

clean:
	rm -f $(TARGET)
	rm -f $(OBJECTS)
	rm -f $(TESTS)
	(cd llhttp && make clean)
