#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "editor/utils.h"
#include "editor/commands.h"
#include "editor/search.h"
#include "editor/defs.h"
#include "editor/keys.h"
#include "editor_row.h"
#include "status_bar.h"
#include "terminal.h"


extern struct Editor editor;


void editorProcessKeypress() {
    int c = editorReadKey();

    if (!(IS_SHIFT_KEY(c) || c == CTRL_KEY('c'))) {
        editor.selecting = false;
    }

    switch (c) {
        case CTRL_KEY('q'):
            editorQuit();
            break;
        case CTRL_KEY('s'):
            editorSave();
            break;
        case CTRL_KEY('f'):
            editorFind();
            break;
        case CTRL_KEY('n'):
            searchResultNext();
            break;
        case CTRL_KEY('p'):
            searchResultPrev();
            break;
        case CTRL_KEY('c'):
            editorCopy();
            break;
        case CTRL_KEY('v'):
            editorPaste();
            break;
        case CTRL_KEY('l'):
            /* TODO */
            break;

        case HOME_KEY:
        case END_KEY:
        case PAGE_UP:
        case PAGE_DOWN:
        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
        case CTRL_ARROW_UP:
        case CTRL_ARROW_DOWN:
        case CTRL_ARROW_LEFT:
        case CTRL_ARROW_RIGHT:
            editingPointMove(c);
            break;

        case SHIFT_ARROW_UP:
        case SHIFT_ARROW_DOWN:
        case SHIFT_ARROW_RIGHT:
        case SHIFT_ARROW_LEFT:
        case CTRL_SHIFT_ARROW_RIGHT:
        case CTRL_SHIFT_ARROW_LEFT:
            if (!editor.selecting) {
                editor.selection_start = editor.editing_point;
                editor.selecting = true;
            }

            editingPointMove(c);
            break;

        case BACKSPACE:
        case DEL_KEY:
            if (c == DEL_KEY) editingPointMove(ARROW_RIGHT);
            editorDeleteChar();
            break;

        case '\r':
            editorInsertNewline();
            break;
        case '\x1b':
            editor.searching = false;
            break;

        default:
            editorInsertChar(c);
            break;
    }
}

int editorReadKey() {
    int nread;
    char c;

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN)
            editorExitError("editor/editor_read_key/read");
    }

    if (c == '\x1b') {
        char seq[5];

        // if we read an escape charcater we immediatly read two more bytes
        // if either of this two reads times out we assume the user
        // just pressed the ESCAPE key
        if (read(STDOUT_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDOUT_FILENO, &seq[1], 1) != 1) return '\x1b';

        if (seq[0] == '[') {
            if (seq[1] >= '0' && seq[1] <= '9') {
                if (read(STDOUT_FILENO, &seq[2], 1) != 1) return '\x1b';
                if (seq[2] == '~') {
                    switch (seq[1]) {
                        case '1': return HOME_KEY;
                        case '3': return DEL_KEY;
                        case '4': return END_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;
                    }
                } else if (seq[2] == ';') {
                    if (read(STDOUT_FILENO, &seq[3], 1) != 1) return '\x1b';
                    if (read(STDOUT_FILENO, &seq[4], 1) != 1) return '\x1b';

                    if (seq[3] == '5') {
                        switch (seq[4]) {
                            case 'A': return CTRL_ARROW_UP;
                            case 'B': return CTRL_ARROW_DOWN;
                            case 'C': return CTRL_ARROW_RIGHT;
                            case 'D': return CTRL_ARROW_LEFT;
                        }
                    } else if (seq[3] == '2') {
                        switch (seq[4]) {
                            case 'A': return SHIFT_ARROW_UP;
                            case 'B': return SHIFT_ARROW_DOWN;
                            case 'C': return SHIFT_ARROW_RIGHT;
                            case 'D': return SHIFT_ARROW_LEFT;
                        }
                    } else if (seq[3] == '6') {
                        switch (seq[4]) {
                            case 'C': return CTRL_SHIFT_ARROW_RIGHT;
                            case 'D': return CTRL_SHIFT_ARROW_LEFT;
                        }
                    }
                }
            } else {
                switch (seq[1]) {
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;
                    case 'F': return END_KEY;
                }
            }
        } else if (seq[0] == 'O') {
            switch (seq[1]) {
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
            }
        }
        return '\x1b';
    } else {
        return c;
    }
}

void editorSetDirty() {
    editor.dirty = true;
    if (editor.search_query != NULL)
        editorSearch(editor.search_query); // search last query
}

void editorExitError(const char *s) {
    WRITE_SEQ(CLEAR_SCREEN);
    WRITE_SEQ(MOVE_CURSOR_TO_ORIG);

    perror(s);
    exit(1);
}

void editorCleanExit() {
    WRITE_SEQ(CLEAR_SCREEN);
    WRITE_SEQ(MOVE_CURSOR_TO_ORIG);
    exit(0);
}

void editorScroll() {
    if (editor.rx < editor.coloff) {
        editor.coloff = editor.rx;
    } else if (editor.rx >= editor.coloff + terminal.screencols) {
        editor.coloff = editor.rx - terminal.screencols + 1;
    }
    if (getRow(editor.editing_point) < editor.rowoff) {
        editor.rowoff = getRow(editor.editing_point);
    } else if (getRow(editor.editing_point) >= editor.rowoff + editor.view_rows) {
        editor.rowoff = getRow(editor.editing_point) - editor.view_rows + 1;
    }
}

void editorCxToRx() {
    editor.rx = 4;  // 4 chars of line number
    unsigned int i;
    for (i = 0; i < getCol(editor.editing_point); i++) {
        if (CURR_ROW.chars[i] == '\t') {
            editor.rx += TAB_SPACE_NUM - 1;
        }
        editor.rx++;
    }
}

void editorRefreshScreen() {
    editorCxToRx();
    editorScroll();

    struct DynamicBuffer dbuf = DBUF_INIT;
    // hide cursor while drawing on screen
    dbuf_append(&dbuf, HIDE_CURSOR_SEQ, HIDE_CURSOR_SEQ_SIZE);
    // ensure cursor is positioned top-left
    dbuf_append(&dbuf, MOVE_CURSOR_TO_ORIG_SEQ, MOVE_CURSOR_TO_ORIG_SEQ_SIZE);

    editorDrawRows(&dbuf);
    infoBarDraw(&dbuf);
    messageBarDraw(&dbuf);

    char buf[32];
    // move cursor to terminal cursor position
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", getRow(editor.editing_point) - editor.rowoff + 1, editor.rx - editor.coloff + 1);
    dbuf_append(&dbuf, buf, strlen(buf));

    // show cursor
    dbuf_append(&dbuf, SHOW_CURSOR_SEQ, SHOW_CURSOR_SEQ_SIZE);

    write(STDOUT_FILENO, dbuf.b, dbuf.len);
    dbuf_free(&dbuf);
}

char* editorRowsToString(int* buflen) {
    *buflen = 0;
    for (unsigned int i = 0; i < editor.numrows; i++) {
        *buflen += editor.rows[i].size + 1;
    }

    char* buf = malloc(*buflen);
    if (buf == NULL) {
        *buflen = 0;
        return NULL;
    }

    char* p = buf;
    for (unsigned int i = 0; i < editor.numrows; i++) {
        memcpy(p, editor.rows[i].chars, editor.rows[i].size);
        p += editor.rows[i].size;
        *p = '\n';
        p++;
    }

    return buf;
}

int editorInsertRow(unsigned int pos, char *s, size_t len) {
    if (pos > editor.numrows) return -1;

    char *new = realloc(editor.rows, sizeof(struct EditorRow) * (editor.numrows + 1));

    if (new == NULL)
        return -1;

    editor.rows = (struct EditorRow*)new;
    memmove(&editor.rows[pos + 1], &editor.rows[pos], sizeof(struct EditorRow) * (editor.numrows - pos));

    editor.rows[pos].size = len;
    editor.rows[pos].chars = malloc(len + 1);
    memcpy(editor.rows[pos].chars, s, len);
    editor.rows[pos].chars[len] = '\0';

    editor.rows[pos].rsize = 0;
    editor.rows[pos].render = NULL;
    editor.rows[pos].hl = NULL;
    editor.rows[pos].search_match_pos = ARRAY_NEW(ArrayUnsignedInt);
    editor.numrows += 1;

    return 0;
}

void editorDeleteRow(unsigned int pos) {
    if (pos > editor.numrows) return;
    editorRowFree(&editor.rows[pos]);
    if (pos != editor.numrows) {
        memmove(&editor.rows[pos], &editor.rows[pos + 1], sizeof(struct EditorRow) * (editor.numrows - pos - 1));
    }
    editor.numrows--;
    editorSetDirty();
}



void editorDrawRow(unsigned int filerow, struct DynamicBuffer* dbuf) {
    editorRowRender(filerow);

    struct EditorRow* row = &editor.rows[filerow];

    // Line number
    char buf[32];
    int len = sprintf(buf, "\x1b[30;100m% 4d", filerow + 1);
    dbuf_append(dbuf, buf, len);

    if (row->rsize == 0) return;

    unsigned int j = 0;
    unsigned int printable_chars = 0;
    char first_hl[10];
    // I'm assuming that the firts thing in a line is a color escape sequence
    memcpy(first_hl, row->render, COLOR_SEQ_SIZE);

    while (j < row->rsize) {
        char c = row->render[j];

        if (printable_chars < editor.coloff) {
            if (c == '\x1b') {
                memcpy(first_hl, &row->render[j], COLOR_SEQ_SIZE);
                j += COLOR_SEQ_SIZE;
            } else {
                printable_chars++;
                j++;
            }
            continue;
        } else if (printable_chars == editor.coloff) {
            dbuf_append(dbuf, first_hl, COLOR_SEQ_SIZE);
        }

        if ((int)(printable_chars - editor.coloff) == terminal.screencols) break;

        if (c == '\x1b') {
            dbuf_append(dbuf, &row->render[j], COLOR_SEQ_SIZE);
            j += COLOR_SEQ_SIZE;
        } else {
            dbuf_append(dbuf, &c, 1);
            printable_chars++;
            j++;
        }
    }
}

void editorDrawRows(struct DynamicBuffer *dbuf) {
    int y;
    for (y = 0; y < editor.view_rows; y++) {
        unsigned int filerow = y + editor.rowoff;
        if (filerow >= editor.numrows) {
            if (editor.numrows == 0 && y == editor.view_rows / 3) {
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome), "Kilo editor -- version %s", KILO_VERSION);
                if (welcomelen > terminal.screencols) {
                    welcomelen = terminal.screencols;
                }

                int padding = (terminal.screencols - welcomelen) / 2;
                if (padding) {
                    dbuf_append(dbuf, "~", 1);
                    padding--;
                }
                while (padding--) {
                    dbuf_append(dbuf, " ", 1);
                }
                dbuf_append(dbuf, welcome, welcomelen);
            } else {
                dbuf_append(dbuf, "~", 1);
            }
        } else {
            editorDrawRow(filerow, dbuf);
        }

        dbuf_append(dbuf, "\x1b[039;049m", COLOR_SEQ_SIZE);
        // erase the part of the line to the right of the cursor:
        // we erase all that is remained after drawing the line
        dbuf_append(dbuf, CLEAR_LINE_CURSOR_TO_END_SEQ, CLEAR_LINE_CURSOR_TO_END_SEQ_SIZE);

        if (y < editor.view_rows -1) {
            dbuf_append(dbuf, "\r\n", 2);
        }
    }
}
