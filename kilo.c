// *** includes ***
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

// *** defines ***
#define CTRL_KEY(k)    ((k) & 0x1f)

// *** data ***

struct editor_config {
    int screenrows;
    int screencols;
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

char editor_read_key() {
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) {
            die("read");
        }
    }
    return c;
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
        ab_append(ab, "~", 1);

        if (y < e_conf.screenrows -1) {
            ab_append(ab, "\r\n", 2);
        }
    }
}

void editor_refresh_screen() {
    struct abuf ab = ABUF_INIT;
    // clear entire screen
    ab_append(&ab, "\x1b[2J", 4);
    // ensure cursor is positioned top-left
    ab_append(&ab, "\x1b[H", 3);

    editor_draw_rows(&ab);

    ab_append(&ab, "\x1b[H", 3);

    write(STDOUT_FILENO, ab.b, ab.len);
    ab_free(&ab);
}

// *** input ***
void editor_process_keypress() {
    char c = editor_read_key();

    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
    }
}

// *** init ***

void init_editor() {
    if (get_window_size(&e_conf.screenrows, &e_conf.screencols) == -1) {
        die("get_window_size");
    }
}

int main() {
    enable_raw_mode();
    init_editor();

    while (1) {
        editor_refresh_screen();
        editor_process_keypress();
    }
    return 0;
}
