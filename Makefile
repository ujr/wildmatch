
CFLAGS = -Wall -Wextra -g

all: recursive

recursive: recursive.c
	$(CC) $(CFLAGS) -o $@ recursive.c

clean:
	rm -f recursive
