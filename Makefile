SHARED_FLAGS = -Wall -Wextra -pedantic -Isrc
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

EXE = loki

loki: $(OBJDIR) $(OBJS)
	$(CC) $(OBJS) -o loki

loki-zig: $(SRCS)
	zig build-exe $(SRCS) -I inc --library c -femit-bin=loki


release: CFLAGS = $(SHARED_FLAGS) -O3
release: clean
release: $(EXE)

test: $(TESTDIR)/*.c
	zig run -I inc --library c $< $(filter-out $(SRCDIR)/$(EXE).c, $(SRCS))

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	gcc $(CFLAGS) -c $< -o $@

$(OBJDIR)/editor_%.o: $(SRCDIR)/editor/%.c
	gcc $(CFLAGS) -c $< -o $@

$(OBJDIR)/utils_%.o: $(SRCDIR)/utils/%.c
	gcc $(CFLAGS) -c $< -o $@

$(OBJDIR)/utils_pre_%.c: $(SRCDIR)/utils/%.c
	$(CC) $(CFLAGS) -E $< -o $@


clean:
	rm -f $(EXE)
	rm -f bin/*

