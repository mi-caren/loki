CC = gcc
SHARED_FLAGS = -Wall -Wextra -pedantic -Iinc
CFLAGS = $(SHARED_FLAGS) -g

SRCDIR=src
OBJDIR=bin
SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

TESTDIR = tests
TESTS = $(wildcard $(TESTDIR)/*.c)
TESTBINS = $(patsubst $(TESTDIR)/%.c, $(TESTDIR)/bin/%, $(TESTS))

kilo: $(OBJDIR) $(OBJS)
	$(CC) $(OBJS) -o $@

release: CFLAGS = $(SHARED_FLAGS) -O3
release: clean
release: kilo

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@


# TESTS
test: $(OBJDIR) $(OBJS) $(TESTDIR)/bin $(TESTBINS)
	@for test in $(TESTBINS) ; do ./$$test ; done

$(TESTDIR)/bin:
	mkdir -p $(TESTDIR)/bin

$(TESTDIR)/bin/%: $(TESTDIR)/%.c
	$(CC) $(CFLAGS) $< $(filter-out $(OBJDIR)/kilo.o, $(OBJS)) -o $@


pre:
	$(CC) $(CFLAGS) $(SRCS) -E

$(OBJDIR)/static-kilo: $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -static -o $@



clean:
	rm -f kilo
	rm -f bin/*

