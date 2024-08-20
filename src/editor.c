#include <bits/types/struct_iovec.h>
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE


#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "editing_point.h"
#include "editor.h"
#include "editor_row.h"
#include "terminal.h"
#include "utils.h"
#include "status_bar.h"



#define KILO_VERSION     "0.0.1"


extern struct Editor editor;
extern struct Terminal terminal;
extern void die(const char *s);


// static void editorRxToCx();
void editor_cx_to_rx();

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



static int editorInsertRow(unsigned int pos, char *s, size_t len)
{
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
    if (editorRowRender(&editor.rows[pos]) != 0)
        return -1;
    editor.numrows += 1;

    return 0;
}

void editorDeleteRow(unsigned int pos)
{
    if (pos > editor.numrows) return;
    editorRowFree(&editor.rows[pos]);
    if (pos != editor.numrows) {
        memmove(&editor.rows[pos], &editor.rows[pos + 1], sizeof(struct EditorRow) * (editor.numrows - pos - 1));
    }
    editor.numrows--;
    editor.dirty = true;
}

// *** editor operations ***

void editorInsertChar(char c) {
    if (editor.numrows == 0) {
        editorInsertRow(editor.numrows, "", 0);
    }
    editorRowInsertChar(&CURR_ROW, editor.editing_point.cx, c);
    editor.editing_point.cx++;
}

void editorInsertNewline() {
    if (editor.numrows == 0) {
        editorInsertRow(0, "", 0);
    }

    editorInsertRow(editor.editing_point.cy + 1, &CURR_ROW.chars[editor.editing_point.cx], CURR_ROW.size - editor.editing_point.cx);
    CURR_ROW.chars[editor.editing_point.cx] = '\0';
    CURR_ROW.size = editor.editing_point.cx;
    editorRowRender(&CURR_ROW);

    editor.editing_point.cy++;
    editor.editing_point.cx = 0;
}

void editorDeleteChar() {
    if (editor.numrows == 0) return;
    if (editor.editing_point.cx == 0 && editor.editing_point.cy == 0) return;

    if (editor.editing_point.cx > 0) {
        editorRowDeleteChar(&CURR_ROW, editor.editing_point.cx - 1);
        editor.editing_point.cx--;
    } else {
        editingPointMove(ARROW_LEFT);
        if (!editorRowAppendString(&CURR_ROW, NEXT_ROW.chars, NEXT_ROW.size)) {
            messageBarSet("Unable to delete char at %d, %d", editor.editing_point.cx - 1, editor.editing_point.cy);
        }
        editorDeleteRow(editor.editing_point.cy + 1);
    }
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
        memcpy(p, editor.rows[i].chars, editor.rows[i].size);
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
/*
    Returns a bool indicating if saving of file has been successfull.
*/
bool editorSave() {
    bool ok = false;

    if (editor.filename == NULL) {
        char* filename = messageBarPrompt("Save as", NULL);
        if (filename == NULL) return false;    // the printing of the error message is handled by messageBarPrompt
        editor.filename = filename;
    }

    int len;
    char* buf = editorRowsToString(&len);
    if (buf == NULL) goto end;

    int fd = open(editor.filename, O_RDWR | O_CREAT, 0644);
    if (fd == -1) goto clean_buf;
    if (ftruncate(fd, len) == -1) goto clean_fd;
    if (write(fd, buf, len) != len) goto clean_fd;
    ok = true;
    editor.dirty = false;

clean_fd:
    close(fd);
clean_buf:
    free(buf);
end:
    if (ok) {
        messageBarSet("%d bytes written", len);
    } else {
        messageBarSet("Unable to save buffer! I/O error: %s", strerror(errno));
    }

    return ok;
}

/***** find *****/
static void searchResultNext() {
    if (editor.search_result.len == 0 || editor.search_result.editing_points == NULL) return;

    size_t next_match = editor.search_result.curr == editor.search_result.len - 1 ? 0 : editor.search_result.curr + 1;
    editor.editing_point = editor.search_result.editing_points[next_match];
    editor.search_result.curr = next_match;
}

static void searchResultPrev() {
    if (editor.search_result.len == 0 || editor.search_result.editing_points == NULL) return;

    size_t prev_match = editor.search_result.curr == 0 ? editor.search_result.len - 1 : editor.search_result.curr - 1;
    editor.editing_point =editor.search_result.editing_points[prev_match];
    editor.search_result.curr = prev_match;
}

static int editingPointPush(struct EditingPoint editing_point) {
    if (editor.search_result.editing_points == NULL) {
        struct EditingPoint* new = malloc(sizeof(struct EditingPoint) * editor.search_result.size);
        if (new == NULL) {
            return -1;
        }
        editor.search_result.editing_points = new;
    }

    if (editor.search_result.len == editor.search_result.size - 1) {
        editor.search_result.size *= 2;
        struct EditingPoint* new = realloc(editor.search_result.editing_points, sizeof(struct EditingPoint) * editor.search_result.size);
        if (new == NULL) {
            return -1;
        }
        editor.search_result.editing_points = new;
    }

    editor.search_result.editing_points[editor.search_result.len] = editing_point;
    editor.search_result.len++;

    return 0;
}

int editorFindCallback(char* query, int key) {
    if (key == ARROW_RIGHT || key == ARROW_DOWN || key == CTRL_KEY('n')) {
        searchResultNext();
        return 0;
    } else if (key == ARROW_LEFT || key == ARROW_UP || key == CTRL_KEY('p')) {
        searchResultPrev();
        return 0;
    } else if (key == '\x1b' || key == '\r' || iscntrl(key) || key > 127) {
        return 0;
    }

    // c is a character so i need to perform another search
    editor.search_result.len = 0;
    editor.search_result.curr = 0;
    int match_index_after_editing_point = -1;

    for (unsigned int i = 0; i < editor.numrows; i++) {
        char* match = NULL;
        int last_pos = 0;
        while ((match = strstr(&editor.rows[i].chars[last_pos], query)) != NULL) {
            // editorPushMatch();
            struct EditingPoint editing_point = (struct EditingPoint){
                .cx =  match - editor.rows[i].chars,
                .cy = i,
            };
            if (editingPointPush(editing_point) == -1) {
                messageBarSet("Unable to push match result");
                return -1;
            }

            last_pos = LAST_SEARCH_RESULT.cx + 1;

            if (
                match_index_after_editing_point == -1
                && (
                    (
                        LAST_SEARCH_RESULT.cy == editor.editing_point.cy
                        && LAST_SEARCH_RESULT.cx >= editor.editing_point.cx
                    ) || LAST_SEARCH_RESULT.cy > editor.editing_point.cy
                )
            ) {
                match_index_after_editing_point = editor.search_result.len - 1;
            }
        }
    }

    // nextNearestMatch
    if (match_index_after_editing_point != -1) {
        editor.editing_point = editor.search_result.editing_points[match_index_after_editing_point];
    } else if (editor.search_result.len > 0) {
        editor.editing_point = editor.search_result.editing_points[0];
    }

    return 0;
}

void editorFind() {
    struct EditingPoint prev_editing_point = editor.editing_point;
    unsigned int prev_coloff = editor.coloff;
    unsigned int prev_rowoff = editor.rowoff;

    char* query = messageBarPrompt("Search", editorFindCallback);

    if (query) {
        free(query);
    } else {
        editor.editing_point = prev_editing_point;
        editor.coloff = prev_coloff;
        editor.rowoff = prev_rowoff;
    }
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


void editor_refresh_screen() {
    editor_cx_to_rx();
    editor_scroll();

    struct DynamicBuffer dbuf = DBUF_INIT;

    // hide cursor while drawing on screen
    dbuf_append(&dbuf, HIDE_CURSOR_SEQ, HIDE_CURSOR_SEQ_SIZE);

    // ensure cursor is positioned top-left
    dbuf_append(&dbuf, MOVE_CURSOR_TO_ORIG_SEQ, MOVE_CURSOR_TO_ORIG_SEQ_SIZE);

    editor_draw_rows(&dbuf);
    infoBarDraw(&dbuf);
    messageBarDraw(&dbuf);

    char buf[32];
    // move cursor to terminal cursor position
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", editor.editing_point.cy - editor.rowoff + 1, editor.rx - editor.coloff + 1);
    dbuf_append(&dbuf, buf, strlen(buf));

    // show cursor
    dbuf_append(&dbuf, SHOW_CURSOR_SEQ, SHOW_CURSOR_SEQ_SIZE);

    write(STDOUT_FILENO, dbuf.b, dbuf.len);
    dbuf_free(&dbuf);
}

// *** input ***


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


static void editorCleanExit()
{
    WRITE_SEQ(CLEAR_SCREEN);
    WRITE_SEQ(MOVE_CURSOR_TO_ORIG);
    exit(0);
}

static void editorQuit()
{
    bool quit = true;

    if (editor.dirty) {
        if (messageBarPromptYesNo("File has unsaved changes. Do you want to save before exiting?")) {
            if (!editorSave()) {
                quit = false;
            }
        }
    }

    if (quit) editorCleanExit();
}

void editor_process_keypress() {
    int c = editor_read_key();

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

        case BACKSPACE:
        case DEL_KEY:
            if (c == DEL_KEY) editingPointMove(ARROW_RIGHT);
            editorDeleteChar();
            break;

        case '\r':
            editorInsertNewline();
            break;
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
}

void editor_run() {
    messageBarSet("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find");
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
    editor.message_bar_buf[0] = '\0';
    editor.message_bar_time = 0;
    editor.dirty = false;

    editor.search_result = (SearchResult){
        .editing_points = NULL,
        .size = DEFAULT_EDITING_POINTS_SIZE,
        .len = 0,
        .curr = 0,
    };

    if (height < 0) {
        editor.view_rows = 0;
    } else if (height <= 2) {
        editor.view_rows = height;
    }
    editor.view_rows = height - 2;
}
