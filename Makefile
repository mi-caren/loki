CFLAGS = -Wall -Wextra -pedantic -Iinc -g
SRCS = src/kilo.c src/terminal.c src/utils.c src/editor.c src/editor_row.c src/editing_point.c src/status_bar.c

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

