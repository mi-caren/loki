CC = gcc
SHARED_FLAGS = -Wall -Wextra -pedantic -Iinc
CFLAGS = $(SHARED_FLAGS) -g
SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, bin/%.o, $(SRCS))


kilo: $(OBJS)
	$(CC) $(OBJS) -o $@


release: CFLAGS = $(SHARED_FLAGS) -O3
release: clean
release: kilo

bin/%.o: src/%.c
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

