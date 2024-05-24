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
void die(Error err) {
    WRITE_SEQ(CLEAR_SCREEN);
    WRITE_SEQ(MOVE_CURSOR_TO_ORIG);

    printf("ERROR CODE %d, MSG: %s\n\r", err.code, err.message);
    exit(1);
}


int main(int argc, char *argv[]) {
    UNWRAP(init_editor(), void);

    if (argc >= 2) {
        UNWRAP(editor_open(argv[1]), void);
    }

    editor_run();

    return 0;
}
