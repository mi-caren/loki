#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "editor/utils.h"
#include "editing_point.h"
#include "editor.h"
#include "editor/commands.h"
#include "editor/search.h"
#include "editor/defs.h"
#include "editor/keys.h"
#include "editor_row.h"
#include "status_bar.h"
#include "terminal.h"
#include "aeolus/string.h"
#include "aeolus/vec.h"

extern struct Editor editor;

void editorProcessKeypress() {
    int c = editorReadKey();

    switch (c) {
        case CTRL_KEY('q'):
            cmdQuit();
            break;
        case CTRL_KEY('s'):
            cmdSave();
            break;
        case CTRL_KEY('f'):
            cmdFind();
            break;
        case CTRL_KEY('n'):
            cmdSearchNext();
            break;
        case CTRL_KEY('p'):
            cmdSearchPrev();
            break;
        case CTRL_KEY('c'):
            cmdCopy();
            break;
        case CTRL_KEY('v'):
            cmdPaste();
            break;
        case CTRL_KEY('x'):
            cmdCut();
            break;
        case CTRL_KEY('z'):
            cmdUndo();
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
            editor.selecting = false;
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
            cmdDelete(c == DEL_KEY);
            break;

        // case '\r':
        //     editorInsertNewline();
        //     editor.selecting = false;
        //     break;
        case '\x1b':
            editor.searching = false;
            editor.selecting = false;
            break;

        default:
            cmdInsertChar(c);
            editor.selecting = false;
            break;
    }
}

int editorReadKey() {
    int nread;
    char c;

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN)
            die("editor/editor_read_key/read");
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

static unsigned int rowNumberColumnWidth() {
    unsigned int numrows = editor.rows->len;
    unsigned int digits = 1;
    while ((numrows /= 10) != 0) {
        digits++;
    }
    if (digits == 1) return 2;

    return digits;
}

void editorCxToRx() {
    editor.rx = rowNumberColumnWidth();
    unsigned int i;
    for (i = 0; i < getCol(editor.editing_point); i++) {
        if (str_char_at(&CURR_ROW->chars, i) == '\t') {
            editor.rx += TAB_SPACE_NUM - 1;
        }
        editor.rx++;
    }
}

void editorRefreshScreen() {
    editorCxToRx();
    editorScroll();

    String buf = str_new();

    // hide cursor while drawing on screen
    str_append(&buf, HIDE_CURSOR_SEQ);
    // ensure cursor is positioned top-left
    str_append(&buf, MOVE_CURSOR_TO_ORIG_SEQ);

    editorDrawRows(&buf);
    infoBarDraw(&buf);
    messageBarDraw(&buf);

    char seq_buf[32];
    // move cursor to terminal cursor position
    snprintf(seq_buf, sizeof(seq_buf), "\x1b[%d;%dH", getRow(editor.editing_point) - editor.rowoff + 1, editor.rx - editor.coloff + 1);
    str_append(&buf, seq_buf);

    // show cursor
    str_append(&buf, SHOW_CURSOR_SEQ);

    write(STDOUT_FILENO, str_chars(&buf), str_len(&buf));
    str_free(&buf);
}

String editorRowsToString() {
    size_t buflen = 0;
    for (EACH(row, editor.rows)) {
        buflen += str_len(&row->chars) + 1;
    }

    String buf = str_new_with_cap(buflen);
    // if (buf == NULL)
    //     return NULL;

    for (EACH(row, editor.rows)) {
        str_appends(&buf, &row->chars);
        str_appendc(&buf, '\n');
    }

    return buf;
}

int editorInsertRow(unsigned int pos, char *s) {
    if (pos > editor.rows->len)
        return -1;

    String str = str_from(s);
    EditorRow row = {
        .chars = str,
        .render = str_new(),
        .hl = vec_new(Highlight),
        .search_match_pos = vec_new(unsigned int),
    };

    vec_insert(editor.rows, row, pos);

    return 0;
}

void editorDeleteRow(unsigned int pos) {
    if (pos >= editor.rows->len) return;

    editorRowFree(vec_get(editor.rows, pos));
    vec_remove(editor.rows, pos);
    editorSetDirty();
}



void editorDrawRow(unsigned int filerow, String* buf) {
    editorRowRender(filerow);

    EditorRow* row = vec_get(editor.rows, filerow);

    // Line number
    char fmt_string[32];
    sprintf(fmt_string, "\x1b[30;100m%%%dd", rowNumberColumnWidth());

    char row_number_buf[32];
    sprintf(row_number_buf, fmt_string, filerow + 1);
    str_append(buf, row_number_buf);

    if (str_len(&row->render) == 0) return;

    unsigned int j = 0;
    unsigned int printable_chars = 0;
    char first_hl[COLOR_SEQ_SIZE + 1];
    // I'm assuming that the firts thing in a line is a color escape sequence
    memcpy(first_hl, str_chars(&row->render), COLOR_SEQ_SIZE);
    first_hl[COLOR_SEQ_SIZE] = '\0';

    while (j < str_len(&row->render)) {
        char c = str_char_at(&row->render, j);

        if (printable_chars < editor.coloff) {
            if (c == '\x1b') {
                memcpy(first_hl, &str_chars(&row->render)[j], COLOR_SEQ_SIZE);
                j += COLOR_SEQ_SIZE;
            } else {
                printable_chars++;
                j++;
            }
            continue;
        } else if (printable_chars == editor.coloff) {
            str_append(buf, first_hl);
        }

        if ((int)(printable_chars - editor.coloff) == terminal.screencols) break;

        if (c == '\x1b') {
            char seq[COLOR_SEQ_SIZE + 1];
            memcpy(seq, &str_chars(&row->render)[j], COLOR_SEQ_SIZE);
            seq[COLOR_SEQ_SIZE] = '\0';
            str_append(buf, seq);
            j += COLOR_SEQ_SIZE;
        } else {
            str_appendc(buf, c);
            printable_chars++;
            j++;
        }
    }
}

void editorDrawRows(String* buf) {
    unsigned int y;
    for (y = 0; y < editor.view_rows; y++) {
        unsigned int filerow = y + editor.rowoff;
        if (filerow >= editor.rows->len) {
            if (editor.rows->len == 0 && y == editor.view_rows / 3) {
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome), "Loki editor -- version %s", LOKI_VERSION);
                if (welcomelen > terminal.screencols) {
                    welcomelen = terminal.screencols;
                }

                int padding = (terminal.screencols - welcomelen) / 2;
                if (padding) {
                    str_appendc(buf, '~');
                    padding--;
                }
                str_repeat_appendc(buf, ' ', padding);
                str_append(buf, welcome);
            } else {
                str_appendc(buf, '~');
            }
        } else {
            editorDrawRow(filerow, buf);
        }

        str_append(buf, "\x1b[039;049m");

        // erase the part of the line to the right of the cursor:
        // we erase all that is remained after drawing the line
        str_append(buf, CLEAR_LINE_CURSOR_TO_END_SEQ);

        if (y < editor.view_rows -1) {
            str_append(buf, "\r\n");
        }
    }
}
