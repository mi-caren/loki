CFLAGS = -Wall -Wextra -pedantic -Iinc -g
# SRCS = src/kilo.c src/terminal.c src/utils.c src/editor.c src/editor_row.c src/editing_point.c src/status_bar.c
SRCS = $(shell find src -name '*.c')
OBJS = $(patsubst src/%.c,bin/%.o,$(SRCS))


kilo: $(OBJS)
#	@echo "Linking..."
	$(CC) $(OBJS) -o kilo
# $(CC) $(CFLAGS) $(SRCS) -o kilo

bin/%.o: src/%.c
#	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

run: bin/kilo
	./bin/kilo

pre:
	$(CC) $(CFLAGS) $(SRCS) -E

bin/static-kilo: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -static -o $@

clean:
	rm -f kilo
	rm -f bin/*

