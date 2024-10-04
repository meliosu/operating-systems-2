CC=gcc
FLAGS=-Wall -g

OBJECTS=main.o
TARGET=proxy

all: main

main: $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS)

main.o:
	$(CC) $(FLAGS) -c main.c


clean:
	rm *.o $(TARGET)
