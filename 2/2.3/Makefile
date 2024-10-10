CC=gcc
FLAGS=-Wall -g
LIBS=-lpthread

TARGET_DIR=target

all: mutex rwlock spin
	rm *.o

main:
	$(CC) $(FLAGS) -c main.c

mutex: main
	$(CC) $(FLAGS) -DSYNC_MUTEX -c queue.c -o queue-mutex.o
	$(CC) $(FLAGS) -o $(TARGET_DIR)/mutex main.o queue-mutex.o $(LIBS)

rwlock: main
	$(CC) $(FLAGS) -DSYNC_RWLOCK -c queue.c -o queue-rwlock.o
	$(CC) $(FLAGS) -o $(TARGET_DIR)/rwlock main.o queue-rwlock.o $(LIBS)

spinlock: main
	$(CC) $(FLAGS) -DSYNC_SPINLOCK -c queue.c -o queue-spin.o
	$(CC) $(FLAGS) -o $(TARGET_DIR)/spin main.o queue-spin.o $(LIBS)


clean:
	rm *.o $(TARGET_DIR)/*
