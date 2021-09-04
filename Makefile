
CFLAGS = -Wall -Wextra -g

all: recursive iterative tests

recursive: recursive.c
	$(CC) $(CFLAGS) -DSTANDALONE -o $@ recursive.c

iterative: iterative.c
	$(CC) $(CFLAGS) -DSTANDALONE -o $@ iterative.c

tests: tests.c recursive.c iterative.c testing.c testing.h
	$(CC) $(CFLAGS) -o $@ tests.c recursive.c iterative.c testing.c

check: tests
	./tests

clean:
	rm -f recursive iterative tests
