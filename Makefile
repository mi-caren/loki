CC = zig cc
SHARED_FLAGS = -Wall -Wextra -pedantic -Iinc
CFLAGS = $(SHARED_FLAGS) -g

SRCDIR=src
OBJDIR=bin

SRCS = $(wildcard $(SRCDIR)/*.c $(SRCDIR)/*/*.c)
ROOT_SRCS = $(wildcard $(SRCDIR)/*.c)
EDITOR_SRCS = $(wildcard $(SRCDIR)/editor/*.c)

ROOT_OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(ROOT_SRCS))
EDITOR_OBJS = $(patsubst $(SRCDIR)/editor/%.c, $(OBJDIR)/editor_%.o, $(EDITOR_SRCS))
OBJS = $(ROOT_OBJS) $(EDITOR_OBJS)

TESTDIR = tests
TESTS = $(wildcard $(TESTDIR)/*.c)
TESTBINS = $(patsubst $(TESTDIR)/%.c, $(TESTDIR)/bin/%, $(TESTS))

EXE = loki

$(EXE): $(OBJDIR) $(OBJS)
	zig build-exe --library c -target x86_64-linux-musl $(OBJS) -femit-bin=$@
#	$(CC) $(OBJS) -o $@

release: CFLAGS = $(SHARED_FLAGS) -O3
release: clean
release: $(EXE)

force-rebuild: clean
force-rebuild: $(EXE)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	zig build-obj -Iinc --library c -target x86_64-linux-musl $< -femit-bin=$@
#	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/editor_%.o: $(SRCDIR)/editor/%.c
	zig build-obj -Iinc --library c -target x86_64-linux-musl $< -femit-bin=$@
#	$(CC) $(CFLAGS) -c $< -o $@


# TESTS
test: $(OBJDIR) $(OBJS) $(TESTDIR)/bin $(TESTBINS)
	@for test in $(TESTBINS) ; do ./$$test ; done

$(TESTDIR)/bin:
	mkdir -p $(TESTDIR)/bin

$(TESTDIR)/bin/%: $(TESTDIR)/%.c
	$(CC) $(CFLAGS) $< $(filter-out $(OBJDIR)/$(EXE).o, $(OBJS)) -o $@


pre:
	$(CC) $(CFLAGS) $(SRCS) -E

$(OBJDIR)/static-$(EXE): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -static -o $@



clean:
	rm -f $(EXE)
	rm -f bin/*

