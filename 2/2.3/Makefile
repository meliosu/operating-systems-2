CC=gcc
FLAGS=-Wall -g
LIBS=-lpthread
TARGET_DIR=target
TARGETS=mutex rwlock spin

all: $(TARGETS)
	rm *.o

$(TARGETS): %: queue-%.o main.o mkdir
	$(CC) $(FLAGS) -o $(TARGET_DIR)/$@ $< main.o $(LIBS)

mkdir:
	mkdir -p $(TARGET_DIR)

main.o:
	$(CC) $(FLAGS) -c main.c

queue-mutex.o:
	$(CC) $(FLAGS) -DSYNC_MUTEX -c queue.c -o queue-mutex.o

queue-rwlock.o:
	$(CC) $(FLAGS) -DSYNC_RWLOCK -c queue.c -o queue-rwlock.o

queue-spin.o:
	$(CC) $(FLAGS) -DSYNC_SPINLOCK -c queue.c -o queue-spin.o


clean:
	rm -rf $(TARGET_DIR)
