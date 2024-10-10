CC=gcc
FLAGS=-Wall -g
LIBS=-lpthread
TARGET_DIR=target

TARGETS=mutex spin cond sem
COMMON=queue-threads.o queue-common.o

all: $(TARGETS)
	rm *.o

%.o: %.c
	$(CC) $(FLAGS) -c $<

$(TARGETS): %: queue-%.o $(COMMON)
	$(CC) $(FLAGS) -o $(TARGET_DIR)/$@ $< $(COMMON) $(LIBS)


clean:
	rm *.o $(TARGET_DIR)/*
