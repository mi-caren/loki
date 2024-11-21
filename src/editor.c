#include "utils/vec.h"
#include <bits/types/struct_iovec.h>
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE


#include <stdbool.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "editing_point.h"
#include "editor.h"
#include "editor/defs.h"
#include "editor/commands.h"
#include "editor/utils.h"
#include "terminal.h"
#include "status_bar.h"
#include "utils/dbuf.h"


struct Editor editor;



// *** init , open, run ***

void editorInit() {
    if (terminalEnableRawMode() == -1)
        editorExitError("Cannot set raw mode\r\n");

    atexit(terminalDisableRawMode);

    if (terminalInit() == -1)
        editorExitError("Unable to initialize terminal\r\n");

    editor.editing_point = (EditingPoint) 0;
    editor.rx = 0;
    editor.numrows = 0;
    editor.rows = NULL;
    editor.rowoff = 0;
    editor.coloff = 0;
    editor.filename = NULL;
    editor.message_bar_buf[0] = '\0';
    editor.message_bar_time = 0;
    editor.dirty = false;

    editor.searching = false;
    editor.search_query = NULL;

    editor.selecting = false;
    editor.selection_start = 0;
    editor.copy_buf = NULL;

    editor.undoable_command_history = VEC(Command);

    int height = terminal.screenrows;
    if (height < 0) {
        editor.view_rows = 0;
    } else if (height <= 2) {
        editor.view_rows = height;
    }
    editor.view_rows = height - 2;
}

int editorOpen(char *filename) {
    free(editor.filename);
    char *new = strdup(filename);
    if (new == NULL)
        return -1;
    editor.filename = new;

    FILE *fp = fopen(filename, "r");
    if (!fp)
        return -1;

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    int retval = 0;
    while((linelen = getline(&line, &linecap, fp)) != -1) {
        // strip off newline or carriage retur at the end of line
        while (linelen > 0 && (line[linelen - 1] == '\n' ||
                               line[linelen - 1] == '\r')) {
            linelen--;
        }

        if (editorInsertRow(editor.numrows, line, linelen) != 0) {
            retval = -1;
            goto cleanup;
        }
    }

cleanup:
    free(line);
    fclose(fp);
    return retval;
}

void editorRun() {
    messageBarSet("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");
    while (1) {
        editorRefreshScreen();
        editorProcessKeypress();
    }
}

/*
 * Print error message and exit with 1
 */
inline void die(const char *s) {
    editorExitError(s);
}
