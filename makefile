CFLAGS = -Wall -Wextra -pedantic -std=c99
SRCS = kilo.c terminal.c

bin/kilo: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o bin/kilo

clean:
	rm -f bin/*

