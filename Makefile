
CFLAGS = -Wall -Wextra -g

all: recursive tests

recursive: recursive.c
	$(CC) $(CFLAGS) -DSTANDALONE -o $@ recursive.c

tests: tests.c recursive.c testing.c testing.h
	$(CC) $(CFLAGS) -o $@ tests.c recursive.c testing.c

check: tests
	@echo "Running Test Suite..."
	./tests

clean:
	rm -f recursive tests
