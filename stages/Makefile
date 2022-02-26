
all: recursive iterative1 iterative2 iterative3 iterative4 iterative5 iterative6

recursive: recursive.c
	gcc -o $@ -Wall -Wextra -g -Og -DSTANDALONE recursive.c

iterative1: iterative1.c
	gcc -o iterative1 -Wall -Wextra -g -Og -DSTANDALONE iterative1.c

iterative2: iterative2.c
	gcc -o iterative2 -Wall -Wextra -g -Og -DSTANDALONE iterative2.c

iterative3: iterative3.c
	gcc -o iterative3 -Wall -Wextra -g -Og -DSTANDALONE iterative3.c

iterative4: iterative4.c
	gcc -o iterative4 -Wall -Wextra -g -Og -DSTANDALONE iterative4.c

iterative5: iterative5.c
	gcc -o iterative5 -Wall -Wextra -g -Og -DSTANDALONE iterative5.c

iterative6: iterative6.c
	gcc -o iterative6 -Wall -Wextra -g -Og -DSTANDALONE iterative6.c

clean:
	rm -f recursive iterative1 iterative2 iterative3 iterative4 iterative5 iterative6
