include config.mk

SRCS = $(wildcard src/*.c src/*/*.c)
ROOT_SRCS = $(wildcard src/*.c)
EDITOR_SRCS = $(wildcard src/editor/*.c)

ROOT_OBJS = $(patsubst src/%.c, build/%.o, $(ROOT_SRCS))
EDITOR_OBJS = $(patsubst src/editor/%.c, build/editor_%.o, $(EDITOR_SRCS))
OBJS = $(ROOT_OBJS) $(EDITOR_OBJS)

# lib aeolus
AEOLUS_SRCS = $(wildcard src/aeolus/*.c)
AEOLUS_OBJS = $(patsubst src/aeolus/%.c, build/aeolus_%.o, $(AEOLUS_SRCS))
LIBS = build/libaeolus.a

DEPS = $(OBJS:.o=.d) $(AEOLUS_OBJS:.o=.d)

all: CPPFLAGS += -DNDEBUG
all: CFLAGS += -O3
all: loki

debug: CFLAGS += -g
debug: loki


loki: build $(OBJS) $(LIBS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

build:
	mkdir -p build

build/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/editor_%.o: src/editor/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


# ----- libaeolus -----
build/aeolus_%.o: src/aeolus/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

build/libaeolus.a: $(AEOLUS_OBJS)
	$(AR) rcs $@ $^
# ---------------------


install: loki
	mkdir -p $(INSTALL_DIR)
	cp -f loki $(INSTALL_DIR)/bin

uninstall:
	rm -f $(INSTALL_DIR)/bin/loki

# ----- Just preprocessing -----
build/utils_pre_%.c: src/utils/%.c
	$(CC) $(CFLAGS) -E $< -o $@

build/pre_%.c: src/%.c
	$(CC) $(CFLAGS) -E $< -o $@
# ------------------------------


test: tests/*.c
	zig run -I inc --library c $< $(filter-out src/loki.c, $(SRCS))


clean:
	rm -f $(EXE)
	rm -f build/*

-include $(DEPS)

.PHONY: all debug test clean
