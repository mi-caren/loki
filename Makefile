CFLAGS = -Wall -Wextra -pedantic -g
SRCS = kilo.c terminal.c utils.c editor.c editor_row.c

kilo: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o kilo

run: bin/kilo
	./bin/kilo

pre:
	$(CC) $(CFLAGS) $(SRCS) -E

bin/static-kilo: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -static -o $@

clean:
	rm -f kilo

