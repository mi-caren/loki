#include <stdlib.h>
#include <unistd.h>

#include "editor.h"


int main(int argc, char *argv[]) {
    editorInit();

    if (argc >= 2) {
        if (editorOpen(argv[1]) != 0) {
            die("Error while opening file");
        };
    }

    editorRun();

    return EXIT_SUCCESS;
}
