CC=gcc
FLAGS=-Wall -g -O2
LIBS=-lpthread
TARGET_DIR=target

TARGETS=mutex spin cond sem
COMMON=queue-threads.o queue-common.o

all: $(TARGETS)
	rm *.o

%.o: %.c
	$(CC) $(FLAGS) -c $<

$(TARGETS): %: queue-%.o $(COMMON) mkdir
	$(CC) $(FLAGS) -o $(TARGET_DIR)/$@ $< $(COMMON) $(LIBS)

mkdir:
	mkdir -p $(TARGET_DIR)


clean:
	rm -rf $(TARGET_DIR)
