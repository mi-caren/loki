SRCDIR=src
TESTDIR = tests

SRCS = $(wildcard $(SRCDIR)/*.c $(SRCDIR)/*/*.c)

EXE = loki

loki: $(SRCS)
	zig build-exe -I inc --library c $(SRCS) -femit-bin=loki


release: clean
	zig build-exe -I inc --library c -O ReleaseSafe $(SRCS) -femit-bin=loki

test: $(TESTDIR)/*.c
	zig run -I inc --library c $< $(filter-out $(SRCDIR)/$(EXE).c, $(SRCS))

clean:
	rm -f $(EXE)

