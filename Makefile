CC = gcc

SHARED_FLAGS = -Wall -Wextra -pedantic -Isrc --std=c23
CFLAGS = $(SHARED_FLAGS) -g

SRCDIR=src
BUILDDIR=build
TESTDIR = tests

SRCS = $(wildcard $(SRCDIR)/*.c $(SRCDIR)/*/*.c)
ROOT_SRCS = $(wildcard $(SRCDIR)/*.c)
EDITOR_SRCS = $(wildcard $(SRCDIR)/editor/*.c)
UTILS_SRCS = $(wildcard $(SRCDIR)/utils/*.c)

ROOT_OBJS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(ROOT_SRCS))
EDITOR_OBJS = $(patsubst $(SRCDIR)/editor/%.c, $(BUILDDIR)/editor_%.o, $(EDITOR_SRCS))
UTILS_OBJS = $(patsubst $(SRCDIR)/utils/%.c, $(BUILDDIR)/utils_%.o, $(UTILS_SRCS))
OBJS = $(ROOT_OBJS) $(EDITOR_OBJS) $(UTILS_OBJS)


loki: $(BUILDDIR) $(OBJS)
	$(CC) $(OBJS) -o loki


$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(BUILDDIR)/editor_%.o: $(SRCDIR)/editor/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(BUILDDIR)/utils_%.o: $(SRCDIR)/utils/%.c
	$(CC) $(CFLAGS) -c $^ -o $@


release: CFLAGS = $(SHARED_FLAGS) -O3
release: clean
release: loki


# ----- Just preprocessing -----
$(BUILDDIR)/utils_pre_%.c: $(SRCDIR)/utils/%.c
	$(CC) $(CFLAGS) -E $< -o $@

$(BUILDDIR)/pre_%.c: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -E $< -o $@
# ------------------------------


test: $(TESTDIR)/*.c
	zig run -I inc --library c $< $(filter-out $(SRCDIR)/loki.c, $(SRCS))


clean:
	rm -f $(EXE)
	rm -f $(BUILDDIR)/*
