// *** includes ***
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "editor.h"
#include "terminal.h"

// *** defines ***
#define KILO_VERSION    "0.0.1"

#define CTRL_KEY(k)    ((k) & 0x1f)

enum editor_key {
    ARROW_LEFT = 1000,    // avoid conflicts with regular chars
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN,
};

// *** data ***


struct editor_config e_conf;


// *** terminal ***
/*
 * Print error message and exit with 1
 */
void die(const char *s) {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    perror(s);
    exit(1);
}

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

        e_conf.row.size = linelen;
        e_conf.row.chars = malloc(linelen + 1);
        memcpy(e_conf.row.chars, line, linelen);
        e_conf.row.chars[linelen] = '\0';
        e_conf.numrows = 1;
    }

    free(line);
    fclose(fp);
}

// *** append buffer ***

struct abuf {
    char *b;
    int len;
};

#define ABUF_INIT {NULL, 0};

void ab_append(struct abuf *ab, const char *s, int len) {
    char *new = realloc(ab->b, ab->len + len);

    if (new == NULL) return;
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void ab_free(struct abuf *ab) {
    free(ab->b);
}

// *** output ***

void editor_draw_rows(struct abuf *ab) {
    int y;
    for (y = 0; y < e_conf.screenrows; y++) {
        if (y >= e_conf.numrows) {
            if (y == e_conf.screenrows / 3) {
                char welcome[80];
                int welcomelen = snprintf(welcome, sizeof(welcome), "Kilo editor -- version %s", KILO_VERSION);
                if (welcomelen > e_conf.screencols) {
                    welcomelen = e_conf.screencols;
                }

                int padding = (e_conf.screencols - welcomelen) / 2;
                if (padding) {
                    ab_append(ab, "~", 1);
                    padding--;
                }
                while (padding--) {
                    ab_append(ab, " ", 1);
                }
                ab_append(ab, welcome, welcomelen);
            } else {
                ab_append(ab, "~", 1);
            }
        } else {
            int len = e_conf.row.size;
            if (len > e_conf.screencols) len = e_conf.screencols;
            ab_append(ab, e_conf.row.chars, len);
        }
        // erase the part of the line to the right of the cursor:
        // we erase all that is remained after drawing the line
        ab_append(ab, "\x1b[K", 3);

        if (y < e_conf.screenrows -1) {
            ab_append(ab, "\r\n", 2);
        }
    }
}

void editor_refresh_screen() {
    struct abuf ab = ABUF_INIT;

    // hide cursor while drawing on screen
    ab_append(&ab, "\x1b[?25l", 6);

    // ensure cursor is positioned top-left
    ab_append(&ab, "\x1b[H", 3);

    editor_draw_rows(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", e_conf.cy + 1, e_conf.cx + 1);
    ab_append(&ab, buf, strlen(buf));

    // show cursor
    ab_append(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.b, ab.len);
    ab_free(&ab);
}

// *** input ***

void editor_move_cursor(int key) {
    switch (key) {
        case ARROW_UP:
            if (e_conf.cy != 0) {
                e_conf.cy--;
            }
            break;
        case ARROW_LEFT:
            if (e_conf.cx != 0) {
                e_conf.cx--;
            }
            break;
        case ARROW_DOWN:
            if (e_conf.cy < e_conf.screenrows - 1) {
                e_conf.cy++;
            }
            break;
        case ARROW_RIGHT:
            if (e_conf.cx < e_conf.screencols - 1) {
                e_conf.cx++;
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
            e_conf.cx = 0;
            break;
        case END_KEY:
            e_conf.cx = e_conf.screencols - 1;
            break;

        case PAGE_UP:
        case PAGE_DOWN:
            {
                int times = e_conf.screenrows;
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
    e_conf.cx = 0;
    e_conf.cy = 0;
    e_conf.numrows = 0;
    if (get_window_size(&e_conf.screenrows, &e_conf.screencols) == -1) {
        die("get_window_size");
    }
}

int main(int argc, char *argv[]) {
    enable_raw_mode();
    init_editor();
    if (argc >= 2) {
        editor_open(argv[1]);
    }

    while (1) {
        editor_refresh_screen();
        editor_process_keypress();
    }
    return 0;
}
