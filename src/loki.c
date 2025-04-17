#include <stdlib.h>
#include <unistd.h>

#include "editor.h"
#include "editor/defs.h"

extern struct Editor editor;

int main(int argc, char *argv[]) {
    // Editor ed;
    editorInit(&editor);

    if (argc >= 2) {
        if (editorOpen(argv[1]) != 0) {
            die("Error while opening file");
        };
    }

    editorRun();

    return EXIT_SUCCESS;
}
