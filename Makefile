include config.mk

CPPFLAGS = -MMD -Isrc -DPRE_ERRDBG -DPOST_ERRDBG
CFLAGS = -Wall -Wextra -pedantic --std=c23
LDFLAGS = -L$(LIB_DIR) -laeolus

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

.PHONY: release
release: CPPFLAGS += -DNDEBUG
release: CFLAGS += -O3
release: loki

.PHONY: debug
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


.PHONY: install
install: loki
	mkdir -p $(INSTALL_DIR)
	cp -f loki $(INSTALL_DIR)

.PHONY: uninstall
uninstall:
	rm -f $(INSTALL_DIR)/loki

# ----- Just preprocessing -----
build/aeolus_pre_%.c: src/aeolus/%.c
	$(CC) $(CFLAGS) -g -E $< -o $@

build/pre_%.c: src/%.c
	$(CC) $(CFLAGS) -g -E $< -o $@
# ------------------------------


.PHONY: test
test: tests/*.c
	zig run -I inc --library c $< $(filter-out src/loki.c, $(SRCS))


.PHONY: clean
clean:
	rm -f $(EXE)
	rm -f build/*

-include $(DEPS)
