CC=gcc
LIBS=-lpthread
FLAGS=-Wall -g

TARGETS=1.1 1.2a-b 1.2c 1.2d 1.2e 1.2f 1.3a 1.3b 1.4a 1.4b 1.4c 1.5a 1.5b

all: $(TARGETS) libthreads threads-test libuthreads uthreads-test

$(TARGETS): %: %.c
	$(CC) $(FLAGS) -o $@ $< $(LIBS)


libthreads:
	$(CC) $(FLAGS) -fPIC -o threads.o -c threads.c
	$(CC) -shared -o libthreads.so threads.o

threads-test: libthreads
	$(CC) $(FLAGS) -o threads-test threads-test.c -L. -lthreads


libuthreads:
	$(CC) $(FLAGS) -fPIC -o uthreads.o -c uthreads.c
	$(CC) -shared -o libuthreads.so uthreads.o

uthreads-test: libuthreads
	$(CC) $(FLAGS) -o uthreads-test uthreads-test.c -L. -luthreads


clean:
	rm $(TARGETS) *.o *.so threads-test uthreads-test
