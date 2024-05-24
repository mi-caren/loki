CFLAGS = -Wall -Wextra -pedantic
SRCS = kilo.c terminal.c utils.c editor.c

bin/kilo: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o bin/kilo

run: bin/kilo
	./bin/kilo

pre:
	$(CC) $(CFLAGS) $(SRCS) -E

bin/static-kilo: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -static -o $@

clean:
	rm -f bin/*

