CC=gcc
LIBS=-lpthread
FLAGS=-Wall

TARGETS=1.1 1.2a-b 1.2c 1.2d 1.2e 1.2f 1.3a 1.3b 1.4a 1.4b 1.4c 1.5a 1.5b 1.6

all: $(TARGETS)

$(TARGETS): %: %.c
	$(CC) $(FLAGS) -o $@ $< $(LIBS)


clean:
	rm $(TARGETS)
