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


#define KILO_VERSION    "0.0.1"

#define CTRL_KEY(k)    ((k) & 0x1f)


extern struct Editor editor;
extern void die(const char *s);


int editor_read_key() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) {
            die("read");
        }
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

// *** file i/o ***

void editor_open(char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) die("fopen");

    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    linelen = getline(&line, &linecap, fp);

    if (linelen != -1) {
        while (linelen > 0 && (line[linelen - 1] == '\n' ||
                               line[linelen - 1] == '\r')) {
            linelen--;
        }

        editor.row.size = linelen;
        editor.row.chars = malloc(linelen + 1);
        memcpy(editor.row.chars, line, linelen);
        editor.row.chars[linelen] = '\0';
        editor.numrows = 1;
    }

    free(line);
    fclose(fp);
}

// *** output ***

void editor_draw_rows(struct DynamicBuffer *dbuf) {
    int y;
    for (y = 0; y < editor.screenrows; y++) {
        if (y >= editor.numrows) {
            if (y == editor.screenrows / 3) {
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome), "Kilo editor -- version %s", KILO_VERSION);
                if (welcomelen > editor.screencols) {
                    welcomelen = editor.screencols;
                }

                int padding = (editor.screencols - welcomelen) / 2;
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
            int len = editor.row.size;
            if (len > editor.screencols) len = editor.screencols;
            dbuf_append(dbuf, editor.row.chars, len);
        }
        // erase the part of the line to the right of the cursor:
        // we erase all that is remained after drawing the line
        dbuf_append(dbuf, "\x1b[K", 3);

        if (y < editor.screenrows -1) {
            dbuf_append(dbuf, "\r\n", 2);
        }
    }
}

void editor_refresh_screen() {
    struct DynamicBuffer dbuf = DBUF_INIT;

    // hide cursor while drawing on screen
    dbuf_append(&dbuf, "\x1b[?25l", 6);

    // ensure cursor is positioned top-left
    dbuf_append(&dbuf, "\x1b[H", 3);

    editor_draw_rows(&dbuf);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", editor.cy + 1, editor.cx + 1);
    dbuf_append(&dbuf, buf, strlen(buf));

    // show cursor
    dbuf_append(&dbuf, "\x1b[?25h", 6);

    write(STDOUT_FILENO, dbuf.b, dbuf.len);
    dbuf_free(&dbuf);
}

// *** input ***

void editor_move_cursor(int key) {
    switch (key) {
        case ARROW_UP:
            if (editor.cy != 0) {
                editor.cy--;
            }
            break;
        case ARROW_LEFT:
            if (editor.cx != 0) {
                editor.cx--;
            }
            break;
        case ARROW_DOWN:
            if (editor.cy < editor.screenrows - 1) {
                editor.cy++;
            }
            break;
        case ARROW_RIGHT:
            if (editor.cx < editor.screencols - 1) {
                editor.cx++;
            }
            break;
    }
}

void editor_process_keypress() {
    int c = editor_read_key();

    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;

        case HOME_KEY:
            editor.cx = 0;
            break;
        case END_KEY:
            editor.cx = editor.screencols - 1;
            break;

        case PAGE_UP:
        case PAGE_DOWN:
            {
                int times = editor.screenrows;
                while (times--) {
                    editor_move_cursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
                }
            }
            break;

        case ARROW_UP:
        case ARROW_LEFT:
        case ARROW_DOWN:
        case ARROW_RIGHT:
            editor_move_cursor(c);
            break;
    }
}

// *** init ***

void init_editor() {
    editor.cx = 0;
    editor.cy = 0;
    editor.numrows = 0;
    if (get_window_size(&editor.screenrows, &editor.screencols) == -1) {
        die("get_window_size");
    }
}
