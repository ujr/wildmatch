
CFLAGS = -Wall -Wextra -g

all: wildmatch tests

wildmatch: main.c wildmatch.c wildmatch.h
	$(CC) $(CFLAGS) -o $@ main.c wildmatch.c

tests: tests.c wildmatch.c stages/recursive.c testing.c testing.h
	$(CC) $(CFLAGS) -o $@ tests.c wildmatch.c stages/recursive.c testing.c

check: tests
	./tests

clean:
	rm -f wildmatch tests *.o
