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
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    printf("ERROR CODE %d, MSG: %s\n\r", err.code, err.message);
    exit(1);
}


int main(int argc, char *argv[]) {
    UNWRAP(init_editor(), void);
    if (argc >= 2) {
        UNWRAP(editor_open(argv[1]), void);
    }

    while (1) {
        editor_refresh_screen();
        editor_process_keypress();
    }
    return 0;
}
