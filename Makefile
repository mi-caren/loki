SRCDIR=src
TESTDIR = tests

SRCS = $(wildcard $(SRCDIR)/*.c $(SRCDIR)/*/*.c)

EXE = loki

loki: $(SRCS)
	zig build-exe $(SRCS) -I inc --library c -target x86_64-linux-musl -femit-bin=loki


release: clean
	zig build-exe -I inc --library c -O ReleaseSafe $(SRCS) -femit-bin=loki

test: $(TESTDIR)/*.c
	zig run -I inc --library c $< $(filter-out $(SRCDIR)/$(EXE).c, $(SRCS))

clean:
	rm -f $(EXE)

