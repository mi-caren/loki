#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "editor.h"
#include "terminal.h"
#include "utils.h"


struct Editor editor;
struct Terminal terminal;


/*
 * Reimposta gli attributi del terminale allo stato
 * in cui erano
 */
void disable_raw_mode() {
    // TCSAFLUSH, prima di uscire scarta tutti gli input non letti,
    // quindi non tutto ciò che c'è dopo il carattere 'q' non viene
    // più passato al terminale ma viene scartato
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &editor.orig_termios) == -1) {
        fprintf(stderr, "Cannot disable raw mode\r\n");
    }
}

int main(int argc, char *argv[]) {
    // It could be useful to check for the specific return value
    // in case of -2, exit(1) because the program would not work
    // in case of -1, inform the user that it has not been possible to
    // backup the old terminal state
    if (enable_raw_mode(&editor.orig_termios) != 0) {
        fprintf(stderr, "Cannot enter raw mode\r\n");
        exit(1);
    }

    atexit(disable_raw_mode);
    init_editor();

    if (argc >= 2) {
        UNWRAP(editor_open(argv[1]), void);
    }

    editor_run();

    return 0;
}
