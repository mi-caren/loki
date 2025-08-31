CC = gcc

SRCDIR=src
BUILDDIR=build
TESTDIR = tests

CPPFLAGS = -MMD -Isrc
SHARED_FLAGS = -Wall -Wextra -pedantic --std=c23
CFLAGS = $(SHARED_FLAGS) -g
LDFLAGS = -L./$(BUILDDIR) -laeolus

SRCS = $(wildcard $(SRCDIR)/*.c $(SRCDIR)/*/*.c)
ROOT_SRCS = $(wildcard $(SRCDIR)/*.c)
EDITOR_SRCS = $(wildcard $(SRCDIR)/editor/*.c)

ROOT_OBJS = $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(ROOT_SRCS))
EDITOR_OBJS = $(patsubst $(SRCDIR)/editor/%.c, $(BUILDDIR)/editor_%.o, $(EDITOR_SRCS))
OBJS = $(ROOT_OBJS) $(EDITOR_OBJS)

# lib aeolus
AEOLUS_SRCS = $(wildcard $(SRCDIR)/aeolus/*.c)
AEOLUS_OBJS = $(patsubst $(SRCDIR)/aeolus/%.c, $(BUILDDIR)/aeolus_%.o, $(AEOLUS_SRCS))
LIBS = $(BUILDDIR)/libaeolus.a

DEPS = $(OBJS:.o=.d) $(AEOLUS_OBJS:.o=.d)


loki: $(BUILDDIR) $(OBJS) $(LIBS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)


$(BUILDDIR):
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/editor_%.o: $(SRCDIR)/editor/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/aeolus_%.o: $(SRCDIR)/aeolus/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@


$(BUILDDIR)/libaeolus.a: $(AEOLUS_OBJS)
	ar rcs $@ $^


.PHONY: release
release: CFLAGS = $(SHARED_FLAGS) -O3
release: clean
release: loki


# ----- Just preprocessing -----
$(BUILDDIR)/utils_pre_%.c: $(SRCDIR)/utils/%.c
	$(CC) $(CFLAGS) -E $< -o $@

$(BUILDDIR)/pre_%.c: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -E $< -o $@
# ------------------------------


.PHONY: test
test: $(TESTDIR)/*.c
	zig run -I inc --library c $< $(filter-out $(SRCDIR)/loki.c, $(SRCS))


.PHONY: clean
clean:
	rm -f $(EXE)
	rm -f $(BUILDDIR)/*

-include $(DEPS)