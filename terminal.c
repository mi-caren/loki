#include <stdio.h>
#include <unistd.h>

#include "editor.h"
#include "terminal.h"

extern struct Editor editor;
extern struct Terminal terminal;
extern void die(const char *s);



/*
 * Legge gli attributi del terminale,
 * ne modifica alcuni e riscrive gli attributi.
 */
void enable_raw_mode() {
    if (tcgetattr(STDIN_FILENO, &terminal.orig_termios) == -1)
        die("Cannot retrieve terminal attributes\r\n");

    struct termios raw = terminal.orig_termios;
    cfmakeraw(&raw);
    // Impostiamo il numero minimo di carrateri
    // per il read noncanonical, in modo che read non ritorni
    // subito ma aspetti che un carattere venga passato
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
    // scrive il nuovo valore della struct raw
    // TCSAFLUSH specifica quindo devono essere applicate le modifich:
    //    aspetta che tutti gli output siano stati scritti sul terminale
    //    e scarta tutti gli input non letti
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("Cannot set raw mode\r\n");
}

/*
 * Reimposta gli attributi del terminale allo stato
 * in cui erano
 */
void disable_raw_mode() {
    // TCSAFLUSH, prima di uscire scarta tutti gli input non letti,
    // quindi non tutto ciò che c'è dopo il carattere 'q' non viene
    // più passato al terminale ma viene scartato
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal.orig_termios) == -1) {
        die("Cannot disable raw mode\r\n");
    }
}

int get_cursor_position(int *rows, int *cols) {
    char buf[32];
    unsigned int i = 0;

    if (WRITE_SEQ(REQUEST_CURSOR_POSITION) != 4) {
        return -1;
    };

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') {
        return -1;
    };
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) {
        return -1;
    };

    return 0;
}

int terminal_get_window_size(int *rows, int *cols) {
    if (WRITE_SEQ(MOVE_CURSOR_TO_BOTTOM_RIGHT) != 12) {
        return -1;
    };

    return get_cursor_position(rows, cols);
}

void init_terminal() {
    terminal.cursor_pos.cx = 0;
    terminal.cursor_pos.cy = 0;
    if(terminal_get_window_size(&terminal.screenrows, &terminal.screencols) != 0) {
        die("Unable to retrieve window size\r\n");
    }
}

