#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "editor.h"
#include "terminal.h"
#include "utils.h"


struct Editor editor;
struct Terminal terminal;

/*
 * Print error message and exit with 1
 */
void die(const char *s) {
    WRITE_SEQ(CLEAR_SCREEN);
    WRITE_SEQ(MOVE_CURSOR_TO_ORIG);

    perror(s);
    exit(1);
}

int main(int argc, char *argv[]) {
    enable_raw_mode();
    atexit(disable_raw_mode);

    init_terminal();
    init_editor(terminal.screenrows);

    if (argc >= 2) {
        UNWRAP(editor_open(argv[1]), void);
    }

    editor_run();

    return 0;
}
