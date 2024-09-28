#include <stdlib.h>
#include <unistd.h>

#include "editor.h"
#include "terminal.h"
#include "utils.h"


int main(int argc, char *argv[]) {
    enable_raw_mode();
    atexit(disable_raw_mode);

    init_terminal();
    init_editor(terminal.screenrows);

    if (argc >= 2) {
        if (editor_open(argv[1]) != 0) {
            die("Error while opening file");
        };
    }

    editor_run();

    return 0;
}
