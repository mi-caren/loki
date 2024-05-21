#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "editor.h"
#include "terminal.h"
#include "utils.h"


struct editor_config e_conf;


/*
 * Print error message and exit with 1
 */
void die(const char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}


int main(int argc, char *argv[]) {
    enable_raw_mode();
    init_editor();
    if (argc >= 2) {
        editor_open(argv[1]);
    }

    while (1) {
        editor_refresh_screen();
        editor_process_keypress();
    }
    return 0;
}
