CC=gcc
LIBS=-lpthread
FLAGS=-Wall

TARGETS=1.1 1.2a-b 1.2c 1.2d 1.2e 1.2f 1.3a 1.3b 1.4a 1.4b 1.4c 1.5a 1.5b

all: $(TARGETS) 1.6

$(TARGETS): %: %.c
	$(CC) $(FLAGS) -o $@ $< $(LIBS)

1.6:
	$(CC) $(FLAGS) -fPIC -o mythread.o -c 1.6.c
	$(CC) -shared -o libmythread.so mythread.o


1.6-test:
	$(CC) $(FLAGS) -o 1.6-test 1.6-test.c -L. -lmythread
	LD_LIBRARY_PATH=. ./1.6-test && rm 1.6-test



clean:
	rm $(TARGETS) *.o *.so
