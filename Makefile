CC = gcc
SHARED_FLAGS = -Wall -Wextra -pedantic -Isrc --std=c23
CFLAGS = $(SHARED_FLAGS) -g

SRCDIR=src
OBJDIR=bin
TESTDIR = tests

SRCS = $(wildcard $(SRCDIR)/*.c $(SRCDIR)/*/*.c)
ROOT_SRCS = $(wildcard $(SRCDIR)/*.c)
EDITOR_SRCS = $(wildcard $(SRCDIR)/editor/*.c)
UTILS_SRCS = $(wildcard $(SRCDIR)/utils/*.c)

ROOT_OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(ROOT_SRCS))
EDITOR_OBJS = $(patsubst $(SRCDIR)/editor/%.c, $(OBJDIR)/editor_%.o, $(EDITOR_SRCS))
UTILS_OBJS = $(patsubst $(SRCDIR)/utils/%.c, $(OBJDIR)/utils_%.o, $(UTILS_SRCS))
OBJS = $(ROOT_OBJS) $(EDITOR_OBJS) $(UTILS_OBJS)


loki: $(OBJDIR) $(OBJS)
	$(CC) $(OBJS) -o loki


$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(OBJDIR)/editor_%.o: $(SRCDIR)/editor/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

$(OBJDIR)/utils_%.o: $(SRCDIR)/utils/%.c
	$(CC) $(CFLAGS) -c $^ -o $@


release: CFLAGS = $(SHARED_FLAGS) -O3
release: clean
release: loki


# ----- Just preprocessing -----
$(OBJDIR)/utils_pre_%.c: $(SRCDIR)/utils/%.c
	$(CC) $(CFLAGS) -E $< -o $@

$(OBJDIR)/pre_%.c: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -E $< -o $@
# ------------------------------


test: $(TESTDIR)/*.c
	zig run -I inc --library c $< $(filter-out $(SRCDIR)/loki.c, $(SRCS))


clean:
	rm -f $(EXE)
	rm -f bin/*
