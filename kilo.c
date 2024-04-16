// *** includes ***
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

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

typedef struct erow {
    int size;
    char *chars;
} erow;

struct editor_config {
    int cx, cy;
    int screenrows;
    int screencols;
    int numrows;
    erow row;
    struct termios orig_termios;
};

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

/*
 * Reimposta gli attributi del terminale allo stato
 * in cui erano
 */
void disable_raw_mode() {
    // TCSAFLUSH, prima di uscire scarta tutti gli input non letti,
    // quindi non tutto ciò che c'è dopo il carattere 'q' non viene 
    // più passato al terminale ma viene scartato
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &e_conf.orig_termios) == -1) {
        die("disable_raw_mode/tcsetattr");
    }
}

/*
 * Legge gli attributi del terminale,
 * ne modifica alcuni e riscrive gli attributi.
 */
void enable_raw_mode() {
    // Legge gli attributi del terminale nella struct raw
    if (tcgetattr(STDIN_FILENO, &e_conf.orig_termios) == -1) {
        die("enable_raw_mode/tcgetattr");
    }
    // Registriamo una funzione perchè sia chiamata quando
    // il programma termina, o perchè ritorna da main,
    // o perchè viene chiamato exit()
    atexit(disable_raw_mode);
    struct termios raw = e_conf.orig_termios;
    // disabilita l'echoing: ciò che si digita
    // non saràstampato a terminale
    // Flags:
    //    c_lflag -> local flags
    //    c_iflag -> input flags
    //    c_oflag -> output flags
    //    c_cflag -> control flags
    // In canonical mode, the terminal passes the input to the program
    // only when a ENTER is pressed. If we disable it, we read input byte-by-byte
    // disable ICRNL to avoid translating \r to \n
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    // disable output translation of \n to \n\r
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag &= ~(CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    // Impostiamo il numero minimo di carrateri
    // per il read noncanonical, in modo che read non ritorni
    // subito ma aspetti che un carattere venga passato
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    // scrive il nuovo valore della struct raw
    // TCSAFLUSH specifica quindo devono essere applicate le modifich:
    //    aspetta che tutti gli output siano stati scritti sul terminale
    //    e scarta tutti gli input non letti
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        die("enable_raw_mode/tcsetattr");
    }
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

int get_cursor_position(int *rows, int *cols) {
    char buf[32];
    unsigned int i = 0;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;

    return 0;
}

int get_window_size(int *rows, int *cols) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    return get_cursor_position(rows, cols);
}

// *** file i/o ***

void editor_open() {
    char *line = "Hello, World!";
    ssize_t linelen = 13;

    e_conf.row.size = 13;
    e_conf.row.chars = malloc(linelen + 1);
    memcpy(e_conf.row.chars, line, linelen);
    e_conf.row.chars[linelen] = '\0';
    e_conf.numrows = 1;
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

int main() {
    enable_raw_mode();
    init_editor();
    editor_open();

    while (1) {
        editor_refresh_screen();
        editor_process_keypress();
    }
    return 0;
}
