CC=gcc
LIBS=-lpthread
FLAGS=-Wall

TARGETS=1.1 1.2a-b 1.2c 1.2d 1.2e 1.2f

all: $(TARGETS)

1.1:
	$(CC) $(FLAGS) -o 1.1 1.1.c $(LIBS)

1.2a-b:
	$(CC) $(FLAGS) -o 1.2a-b 1.2a-b.c $(LIBS)

1.2c:
	$(CC) $(FLAGS) -o 1.2c 1.2c.c $(LIBS)

1.2d:
	$(CC) $(FLAGS) -o 1.2d 1.2d.c $(LIBS)

1.2e:
	$(CC) $(FLAGS) -o 1.2e 1.2e.c $(LIBS)

1.2f:
	$(CC) $(FLAGS) -o 1.2f 1.2f.c $(LIBS)


clean:
	rm $(TARGETS)
