CFLAGS = -Wall -Wextra -pedantic -std=c99
SRCS = kilo.c terminal.c utils.c editor.c

bin/kilo: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o bin/kilo

run: bin/kilo
	./bin/kilo

pre:
	$(CC) $(CFLAGS) $(SRCS) -E

clean:
	rm -f bin/*
