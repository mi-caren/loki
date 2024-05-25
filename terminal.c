#include <stdio.h>
#include <stdlib.h>
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
int enable_raw_mode(struct termios *orig_termios) {
    if (orig_termios != NULL) {
        // Legge gli attributi del terminale
        if (tcgetattr(STDIN_FILENO, orig_termios) == -1)
            return -1;
    }

    struct termios raw = *orig_termios;
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
        return -2;

    return 0;
}

RESULT(void) get_cursor_position(unsigned int *rows, unsigned int *cols) {
    char buf[32];
    unsigned int i = 0;

    RESULT(void) res = INIT_RESULT;

    if (WRITE_SEQ(REQUEST_CURSOR_POSITION) != 4) {
        ERROR(res, 1, "terminal/get_cursor_position/request_position");
    };

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') {
        ERROR(res, 2, "terminal/get_cursor_position/read_escape_seq");
    };
    if (sscanf(&buf[2], "%u;%u", rows, cols) != 2) {
        ERROR(res, 3, "terminal/get_cursor_position/read_terminal_response");
    };

    return res;
}

RESULT(void) get_window_size(unsigned int *rows, unsigned int *cols) {
    if (WRITE_SEQ(MOVE_CURSOR_TO_BOTTOM_RIGHT) != 12) {
        RESULT(void) res = INIT_RESULT;
        ERROR(res, 1, "terminal/get_window_size/move_cursor_to_bottom_right");
    };
    return get_cursor_position(rows, cols);
}
