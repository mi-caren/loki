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

#include "editor_row.h"
#include "editing_point.h"
#include "editor.h"
#include "editor/defs.h"
#include "editor/commands.h"
#include "editor/utils.h"
#include "terminal.h"
#include "status_bar.h"
#include "utils/vec.h"
#include "utils/result.h"
#include "utils/utils.h"


struct Editor editor;


void _abortHandler() {
    WRITE_SEQ(LEAVE_ALTERNATE_SCREEN);
    write(STDOUT_FILENO, "\r\n", 2);
}

void atPanicCallback() {
    WRITE_SEQ(LEAVE_ALTERNATE_SCREEN);
}

void editorInit(Editor* ed) {
    // We immediately switch to alternate screen
    // because terminalInit function can mess the terminal by moving the cursor
    WRITE_SEQ(ENTER_ALTERNATE_SCREEN);
    // Leave alternate screen if a panic occurs
    atPanic(atPanicCallback);

    if (IS_ERROR(terminalInit()))
        editorExitError("Unable to initialize terminal\n");

    atexit(terminalDeinit);

    ed->editing_point = (EditingPoint) 0;
    ed->rx = 0;
    ed->rows = VEC_NEW(EditorRow);
    ed->rowoff = 0;
    ed->coloff = 0;
    ed->filename = NULL;
    ed->message_bar_buf[0] = '\0';
    ed->message_bar_time = 0;
    ed->dirty = false;

    ed->searching = false;
    ed->search_query = NULL;

    ed->selecting = false;
    ed->selection_start = 0;
    ed->copy_buf = NULL;

    ed->command_history = VEC_NEW(VEC(CoreCommand));
    ed->curr_history_cmd = NULL;

    int height = terminal.screenrows;
    if (height < 0) {
        ed->view_rows = 0;
    } else if (height <= 2) {
        ed->view_rows = height;
    }
    ed->view_rows = height - 2;
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
            line[--linelen] = '\0';
        }

        if (editorInsertRow(vecLen(editor.rows), line) != 0) {
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
