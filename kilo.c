#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "editor.h"
#include "terminal.h"
#include "utils.h"


struct Editor editor;
struct Terminal terminal;

void cleanup() {
    if (terminal_disable_raw_mode() == -1) {
        fprintf(stderr, "Cannot disable raw mode\r\n");
    }
}

int main(int argc, char *argv[]) {
    // It could be useful to check for the specific return value
    // in case of -2, exit(1) because the program would not work
    // in case of -1, inform the user that it has not been possible to
    // backup the old terminal state
    if (enable_raw_mode() != 0) {
        fprintf(stderr, "Cannot enter raw mode\r\n");
        return 1;
    }

    atexit(cleanup);

    if (init_terminal() != 0) {
        fprintf(stderr, "Unable to retrieve window size\r\n");
        return 1;
    }
    init_editor(terminal.screenrows);

    if (argc >= 2) {
        UNWRAP(editor_open(argv[1]), void);
    }

    editor_run();

    return 0;
}
