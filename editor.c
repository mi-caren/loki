#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
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
extern void die(const char *s);

/*
 * Print error message and exit with 1
 */
void die_error(Error err) {
    WRITE_SEQ(CLEAR_SCREEN);
    WRITE_SEQ(MOVE_CURSOR_TO_ORIG);

    printf("ERROR CODE %d, MSG: %s\r\n", err.code, err.message);
    exit(1);
}


int editor_read_key() {
    int nread;
    char c;

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN)
            die("editor/editor_read_key/read");
    }

    if (c == '\x1b') {
        char seq[3];

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

// *** row operations ***

int editor_render_row(struct EditorRow *row) {
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

    if (new == NULL)
        return -1;

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

    return 0;
}

int editor_append_row(char *line, size_t linelen) {
    char *new = realloc(editor.rows, sizeof(struct EditorRow) * (editor.numrows + 1));

    if (new == NULL)
        return -1;

    editor.rows = (struct EditorRow*)new;
    editor.rows[editor.numrows].size = linelen;
    editor.rows[editor.numrows].chars = malloc(linelen + 1);
    memcpy(editor.rows[editor.numrows].chars, line, linelen);
    editor.rows[editor.numrows].chars[linelen] = '\0';

    editor.rows[editor.numrows].rsize = 0;
    editor.rows[editor.numrows].render = NULL;
    if (editor_render_row(&editor.rows[editor.numrows]) != 0)
        return -1;
    editor.numrows += 1;

    return 0;
}

static void editorRowInsertChar(struct EditorRow* row, unsigned int pos, char c) {
    if (pos > row->size)
        pos = row->size;
    row->chars = realloc(row->chars, row->size + 2);
    memmove(&row->chars[pos + 1], &row->chars[pos], row->size - pos + 1);
    row->size++;
    row->chars[pos] = c;
    editor_render_row(row);
}

// *** editor operations ***

void editorInsertChar(char c) {
    if (editor.numrows == 0) {
        editor_append_row("", 0);
    }
    editorRowInsertChar(&CURR_ROW, editor.editing_point.cx, c);
    editor.editing_point.cx++;
}

// *** file i/o ***

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
        memcpy(p, &editor.rows[i].chars, editor.rows[i].size);
        p += editor.rows[i].size;
        *p = '\n';
        p++;
    }

    return buf;
}

int editor_open(char *filename) {
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

        if (editor_append_row(line, linelen) != 0) {
            retval = -1;
            goto cleanup;
        }
    }

cleanup:
    free(line);
    fclose(fp);
    return retval;
}


// The normal way to overwrite a file is to pass the O_TRUNC flag to open(),
// which truncates the file completely, making it an empty file,
// before writing the new data into it.
// By truncating the file ourselves to the same length as the data we are planning to write into it,
// we are making the whole overwriting operation a little bit safer in case
// the ftruncate() call succeeds but the write() call fails.
// In that case, the file would still contain most of the data it had before.
// But if the file was truncated completely by the open() call and then the write() failed, you’d end up with all of your data lost.

// More advanced editors will write to a new, temporary file,
// and then rename that file to the actual file the user wants to overwrite,
// and they’ll carefully check for errors through the whole process.
void editorSave() {
    if (editor.filename == NULL) return;

    int len;
    char* buf = editorRowsToString(&len);

    int fd = open(editor.filename, O_RDWR | O_CREAT, 0644);
    ftruncate(fd, len);
    write(fd, buf, len);

    close(fd);
    free(buf);
}

// *** output ***

void editor_scroll() {
    if (editor.rx < editor.coloff) {
        editor.coloff = editor.rx;
    } else if (editor.rx >= editor.coloff + terminal.screencols) {
        editor.coloff = editor.rx - terminal.screencols + 1;
    }
    if (editor.editing_point.cy < editor.rowoff) {
        editor.rowoff = editor.editing_point.cy;
    } else if (editor.editing_point.cy >= editor.rowoff + editor.view_rows) {
        editor.rowoff = editor.editing_point.cy - editor.view_rows + 1;
    }
}

void editor_draw_rows(struct DynamicBuffer *dbuf) {
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
            int len = saturating_sub(editor.rows[filerow].rsize, editor.coloff);
            if (len > terminal.screencols) len = terminal.screencols;
            dbuf_append(dbuf, &editor.rows[filerow].render[editor.coloff], len);
        }
        // erase the part of the line to the right of the cursor:
        // we erase all that is remained after drawing the line
        dbuf_append(dbuf, CLEAR_LINE_CURSOR_TO_END_SEQ, CLEAR_LINE_CURSOR_TO_END_SEQ_SIZE);

        if (y < editor.view_rows -1) {
            dbuf_append(dbuf, "\r\n", 2);
        }
    }
}

void editor_draw_status_bar(struct DynamicBuffer *dbuf) {
    char buf[32];
    // move cursor to beginning status bar
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", editor.view_rows + 1, 0);
    dbuf_append(dbuf, buf, strlen(buf));

    dbuf_append(dbuf, INVERTED_COLOR_SEQ, INVERTED_COLOR_SEQ_SIZE);

    char status[terminal.screencols];
    int len = snprintf(status, terminal.screencols / 4 * 3, "%s", editor.filename ? editor.filename : "[New Buffer]");
    dbuf_append(dbuf, status, len);
    int len_s2 = snprintf(status, terminal.screencols / 4, "%d/%d lines ", editor.editing_point.cy + (editor.numrows > 0 ? 1 : 0), editor.numrows);


    while (len < (int)(terminal.screencols - len_s2)) {
        dbuf_append(dbuf, " ", 1);
        len++;
    }
    dbuf_append(dbuf, status, len_s2);
    dbuf_append(dbuf, NORMAL_FORMATTING_SEQ, NORMAL_FORMATTING_SEQ_SIZE);
}

void editor_draw_msg_bar(struct DynamicBuffer *dbuf) {
    char buf[32];
    // move cursor to beginning message bar
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", editor.view_rows + 2, 0);
    dbuf_append(dbuf, buf, strlen(buf));
    dbuf_append(dbuf, CLEAR_LINE_CURSOR_TO_END_SEQ, CLEAR_LINE_CURSOR_TO_END_SEQ_SIZE);
    int msglen = strlen(editor.statusmsg);
    if (msglen > terminal.screencols) msglen = terminal.screencols;
    if (msglen && time(NULL) - editor.statusmsg_time < 5)
        dbuf_append(dbuf, editor.statusmsg, msglen);
}

void editor_refresh_screen() {
    struct DynamicBuffer dbuf = DBUF_INIT;

    // hide cursor while drawing on screen
    dbuf_append(&dbuf, HIDE_CURSOR_SEQ, HIDE_CURSOR_SEQ_SIZE);

    // ensure cursor is positioned top-left
    dbuf_append(&dbuf, MOVE_CURSOR_TO_ORIG_SEQ, MOVE_CURSOR_TO_ORIG_SEQ_SIZE);

    editor_draw_rows(&dbuf);
    editor_draw_status_bar(&dbuf);
    editor_draw_msg_bar(&dbuf);

    char buf[32];
    // move cursor to terminal cursor position
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", editor.editing_point.cy - editor.rowoff + 1, editor.rx - editor.coloff + 1);
    dbuf_append(&dbuf, buf, strlen(buf));

    // show cursor
    dbuf_append(&dbuf, SHOW_CURSOR_SEQ, SHOW_CURSOR_SEQ_SIZE);

    write(STDOUT_FILENO, dbuf.b, dbuf.len);
    dbuf_free(&dbuf);
}

void editor_set_status_message(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(editor.statusmsg, sizeof(editor.statusmsg), fmt, ap);
    va_end(ap);
    editor.statusmsg_time = time(NULL);
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

void editor_cx_to_rx() {
    editor.rx = 0;
    unsigned int i;
    for (i = 0; i < editor.editing_point.cx; i++) {
        if (CURR_ROW.chars[i] == '\t') {
            editor.rx += TAB_SPACE_NUM - 1;
        }
        editor.rx++;
    }
}

void editor_process_keypress() {
    int c = editor_read_key();

    switch (c) {
        case CTRL_KEY('q'):
            WRITE_SEQ(CLEAR_SCREEN);
            WRITE_SEQ(MOVE_CURSOR_TO_ORIG);
            exit(0);
            break;
        case CTRL_KEY('h'):
        case CTRL_KEY('l'):
            /* TODO */
            break;
        case CTRL_KEY('s'):
            editorSave();
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
                if (c == PAGE_UP) {
                    editor.editing_point.cy = editor.rowoff;
                } else {
                    editor.editing_point.cy = editor.rowoff + editor.view_rows - 1;
                    if (editor.editing_point.cy > editor.numrows)
                        editor.editing_point.cy = editor.numrows;
                }
                int times = editor.view_rows;
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

        case BACKSPACE:
        case DEL_KEY:
        case '\r':
        case '\x1b':
        // We ignore the Escape key because there are many key escape sequences
        // that we aren’t handling (such as the F1–F12 keys),
        // and the way we wrote editorReadKey(), pressing those keys
        // will be equivalent to pressing the Escape key
            /* TODO */
            break;

        default:
            editorInsertChar(c);
            break;
    }

    editor_cx_to_rx();
    editor_scroll();
}

void editor_run() {
    editor_set_status_message("HELP: Ctrl-Q = quit");
    while (1) {
        editor_refresh_screen();
        editor_process_keypress();
    }
}

// *** init ***

void init_editor(int height) {
    editor.editing_point.cx = 0;
    editor.editing_point.cy = 0;
    editor.rx = 0;
    editor.numrows = 0;
    editor.rows = NULL;
    editor.rowoff = 0;
    editor.coloff = 0;
    editor.filename = NULL;
    editor.statusmsg[0] = '\0';
    editor.statusmsg_time = 0;

    if (height < 0) {
        editor.view_rows = 0;
    } else if (height <= 2) {
        editor.view_rows = height;
    }
    editor.view_rows = height - 2;
}
