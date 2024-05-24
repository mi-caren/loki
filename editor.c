#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "editor.h"
#include "terminal.h"
#include "utils.h"


#define KILO_VERSION     "0.0.1"

#define CTRL_KEY(k)      ((k) & 0x1f)
#define TAB_SPACE_NUM    4


extern struct Editor editor;
extern struct Terminal terminal;


RESULT(int) editor_read_key() {
    int nread;
    char c;
    RESULT(int) res = INIT_RESULT;

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN)
            ERROR(res, 1, "editor/editor_read_key/read");
    }

    if (c == '\x1b') {
        char seq[3];

        // if we read an escape charcater we immediatly read two more bytes
        // if either of this two reads times out we assume the user
        // just pressed the ESCAPE key
        if (read(STDOUT_FILENO, &seq[0], 1) != 1) RETVAL(res, '\x1b');
        if (read(STDOUT_FILENO, &seq[1], 1) != 1) RETVAL(res, '\x1b');

        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDOUT_FILENO, &seq[2], 1) != 1) RETVAL(res, '\x1b');
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '1': RETVAL(res, HOME_KEY);
                        case '3': RETVAL(res, DEL_KEY);
                        case '4': RETVAL(res, END_KEY);
                        case '5': RETVAL(res, PAGE_UP);
                        case '6': RETVAL(res, PAGE_DOWN);
                        case '7': RETVAL(res, HOME_KEY);
                        case '8': RETVAL(res, END_KEY);
                    }
                }
            } else {
                switch (seq[1]) {
                    case 'A': RETVAL(res, ARROW_UP);
                    case 'B': RETVAL(res, ARROW_DOWN);
                    case 'C': RETVAL(res, ARROW_RIGHT);
                    case 'D': RETVAL(res, ARROW_LEFT);
                    case 'H': RETVAL(res, HOME_KEY);
                    case 'F': RETVAL(res, END_KEY);
                }
            }
        } else if (seq[0] == 'O') {
            switch (seq[1]) {
                case 'H': RETVAL(res, HOME_KEY);
                case 'F': RETVAL(res, END_KEY);
            }
        }
        RETVAL(res, '\x1b');
    } else {
        RETVAL(res, c);
    }
}

// *** row operations ***

RESULT(void) editor_render_row(struct EditorRow *row) {
    unsigned int i;
    unsigned int tabs = 0;
    for (i = 0; i < row->size; i++) {
        if (row->chars[i] == '\t')
            tabs++;
    }

    // eventrully free render if it is not null
    // this makes the munction more general because it can be called
    // also to RE-rende a row
    free(row->render);
    char *new = malloc(row->size + 1 + tabs*(TAB_SPACE_NUM - 1));

    RESULT(void) res = INIT_RESULT;
    if (new == NULL)
        ERROR(res, 1, "editor/editor_render_row/malloc");

    row->render = new;

    unsigned int j = 0;
    for (i = 0; i < row->size; i++) {
        if (row->chars[i] == '\t') {
            unsigned int tab_i;
            for (tab_i = 0; tab_i < TAB_SPACE_NUM; tab_i++)
                row->render[j++] = ' ';
        } else {
            row->render[j++] = row->chars[i];
        }
    }

    row->render[j] = '\0';
    row->rsize = j;

    return res;
}

RESULT(void) editor_append_row(char *line, size_t linelen) {
    RESULT(void) res = INIT_RESULT;
    char *new = realloc(editor.rows, sizeof(struct EditorRow) * (editor.numrows + 1));

    if (new == NULL)
        ERROR(res, 1, "editor/editor_append_row/realloc");

    editor.rows = (struct EditorRow*)new;
    editor.rows[editor.numrows].size = linelen;
    editor.rows[editor.numrows].chars = malloc(linelen + 1);
    memcpy(editor.rows[editor.numrows].chars, line, linelen);
    editor.rows[editor.numrows].chars[linelen] = '\0';

    editor.rows[editor.numrows].rsize = 0;
    editor.rows[editor.numrows].render = NULL;
    editor_render_row(&editor.rows[editor.numrows]);
    editor.numrows += 1;

    return res;
}

// *** file i/o ***

RESULT(void) editor_open(char *filename) {
    RESULT(void) res = INIT_RESULT;

    FILE *fp = fopen(filename, "r");
    if (!fp)
        ERROR(res, 1, "editor/editor_open/open_file");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    while((linelen = getline(&line, &linecap, fp)) != -1) {
        // strip off newline or carriage retur at the end of line
        while (linelen > 0 && (line[linelen - 1] == '\n' ||
                               line[linelen - 1] == '\r')) {
            linelen--;
        }

        UNWRAP(editor_append_row(line, linelen), void);
    }

    free(line);
    fclose(fp);

    return res;
}

// *** output ***

void editor_scroll() {
    if (editor.editing_point.cx < editor.coloff) {
        editor.coloff = editor.editing_point.cx;
    } else if (editor.editing_point.cx >= editor.coloff + terminal.screencols) {
        editor.coloff = editor.editing_point.cx - terminal.screencols + 1;
    }
    if (editor.editing_point.cy < editor.rowoff) {
        editor.rowoff = editor.editing_point.cy;
    } else if (editor.editing_point.cy >= editor.rowoff + terminal.screenrows) {
        editor.rowoff = editor.editing_point.cy - terminal.screenrows + 1;
    }
}

void editor_draw_rows(struct DynamicBuffer *dbuf) {
    unsigned int y;
    for (y = 0; y < terminal.screenrows; y++) {
        unsigned int filerow = y + editor.rowoff;
        if (filerow >= editor.numrows) {
            if (editor.numrows == 0 && y == terminal.screenrows / 3) {
                char welcome[80];
                unsigned int welcomelen = snprintf(welcome, sizeof(welcome), "Kilo editor -- version %s", KILO_VERSION);
                if (welcomelen > terminal.screencols) {
                    welcomelen = terminal.screencols;
                }

                int padding = (terminal.screencols - welcomelen) / 2;
                if (padding) {
                    UNWRAP(dbuf_append(dbuf, "~", 1), void);
                    padding--;
                }
                while (padding--) {
                    UNWRAP(dbuf_append(dbuf, " ", 1), void);
                }
                UNWRAP(dbuf_append(dbuf, welcome, welcomelen), void);
            } else {
                UNWRAP(dbuf_append(dbuf, "~", 1), void);
            }
        } else {
            unsigned int len = saturating_sub(editor.rows[filerow].rsize, editor.coloff);
            if (len > terminal.screencols) len = terminal.screencols;
            UNWRAP(dbuf_append(dbuf, &editor.rows[filerow].render[editor.coloff], len), void);
        }
        // erase the part of the line to the right of the cursor:
        // we erase all that is remained after drawing the line
        UNWRAP(dbuf_append(dbuf, CLEAR_LINE_CURSOR_TO_END_SEQ, CLEAR_LINE_CURSOR_TO_END_SEQ_SIZE), void);

        if (y < terminal.screenrows -1) {
            UNWRAP(dbuf_append(dbuf, "\r\n", 2), void);
        }
    }
}

void editor_refresh_screen() {
    struct DynamicBuffer dbuf = DBUF_INIT;

    // hide cursor while drawing on screen
    UNWRAP(dbuf_append(&dbuf, HIDE_CURSOR_SEQ, HIDE_CURSOR_SEQ_SIZE), void);

    // ensure cursor is positioned top-left
    UNWRAP(dbuf_append(&dbuf, MOVE_CURSOR_TO_ORIG_SEQ, MOVE_CURSOR_TO_ORIG_SEQ_SIZE), void);

    editor_draw_rows(&dbuf);

    char buf[32];
    // move cursor to terminal cursor position
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", editor.editing_point.cy - editor.rowoff + 1, editor.editing_point.cx - editor.coloff + 1);
    UNWRAP(dbuf_append(&dbuf, buf, strlen(buf)), void);

    // show cursor
    UNWRAP(dbuf_append(&dbuf, SHOW_CURSOR_SEQ, SHOW_CURSOR_SEQ_SIZE), void);

    write(STDOUT_FILENO, dbuf.b, dbuf.len);
    dbuf_free(&dbuf);
}

// *** input ***



void editor_move_editing_point(int key) {
    if (editor.numrows == 0) {
        return;
    }

    switch (key) {
        case ARROW_UP:
            if (editor.editing_point.cy != 0) {
                editor.editing_point.cy--;
            }
            break;
        case ARROW_LEFT:
            if (editor.editing_point.cx != 0) {
                editor.editing_point.cx--;
            } else if (editor.editing_point.cy != 0) {
                editor.editing_point.cy--;
                editor.editing_point.cx = CURR_ROW.size;
            }
            break;
        case ARROW_DOWN:
            if (editor.editing_point.cy < saturating_sub(editor.numrows, 1)) {
                editor.editing_point.cy++;
            }
            break;
        case ARROW_RIGHT:
            if (editor.editing_point.cx < CURR_ROW.size) {
                editor.editing_point.cx++;
            } else if (editor.editing_point.cy < saturating_sub(editor.numrows, 1)) {
                editor.editing_point.cy++;
                editor.editing_point.cx = 0;
            }
            break;
    }

    if (editor.editing_point.cx > CURR_ROW.size) {
        editor.editing_point.cx = CURR_ROW.size;
    }
}

void editor_process_keypress() {
    int c = UNWRAP(editor_read_key(), int);

    switch (c) {
        case CTRL_KEY('q'):
            WRITE_SEQ(CLEAR_SCREEN);
            WRITE_SEQ(MOVE_CURSOR_TO_ORIG);
            exit(0);
            break;

        case HOME_KEY:
            editor.editing_point.cx = 0;
            break;
        case END_KEY:
            editor.editing_point.cx = CURR_ROW.size;
            break;

        case PAGE_UP:
        case PAGE_DOWN:
            {
                int times = terminal.screenrows;
                while (times--) {
                    editor_move_editing_point(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
                }
            }
            break;

        case ARROW_UP:
        case ARROW_LEFT:
        case ARROW_DOWN:
        case ARROW_RIGHT:
            editor_move_editing_point(c);
            break;
    }

    editor_scroll();
}

void editor_run() {
    while (1) {
        editor_refresh_screen();
        editor_process_keypress();
    }
}

// *** init ***

RESULT(void) init_editor() {
    UNWRAP(enable_raw_mode(), void);
    terminal.cursor_pos.cx = 0;
    terminal.cursor_pos.cy = 0;
    editor.editing_point.cx = 0;
    editor.editing_point.cy = 0;
    editor.numrows = 0;
    editor.rows = NULL;
    editor.rowoff = 0;
    editor.coloff = 0;
    return get_window_size(&terminal.screenrows, &terminal.screencols);
}
