#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "terminal.h"
#include "aeolus/result.h"
#include "error.h"

Terminal terminal;


static RESULT(void) terminalGetCursorPosition(int *rows, int *cols);
static RESULT(void) terminalGetWindowSize(int *rows, int *cols);



RESULT(void) terminalInit() {
    TRY(void, terminalEnableRawMode());
    terminal.cursor_pos.cx = 0;
    terminal.cursor_pos.cy = 0;
    TRY(void, terminalGetWindowSize(&terminal.screenrows, &terminal.screencols));

    return OK(void);
}

/*
 * Legge gli attributi del terminale,
 * ne modifica alcuni e riscrive gli attributi.
 */
RESULT(void) terminalEnableRawMode() {
    if (tcgetattr(STDIN_FILENO, &terminal.orig_termios) == -1)
        return ERROR(void, ERR_TERM_READ_ATTR);

    struct termios raw = terminal.orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
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
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        return ERROR(void, ERR_TERM_WRITE_ATTR);

    return OK(void);
}

/*
 * Reimposta gli attributi del terminale allo stato
 * in cui erano
 */
inline void terminalDisableRawMode() {
    // TCSAFLUSH, prima di uscire scarta tutti gli input non letti,
    // quindi non tutto ciò che c'è dopo il carattere 'q' non viene
    // più passato al terminale ma viene scartato
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal.orig_termios);
}

void terminalDeinit() {
    WRITE_SEQ(LEAVE_ALTERNATE_SCREEN);
    terminalDisableRawMode();
}



/* ----------------- STATIC ----------------- */

static RESULT(void) terminalGetCursorPosition(int *rows, int *cols) {
    char buf[32];
    unsigned int i = 0;

    if (WRITE_SEQ(REQUEST_CURSOR_POSITION) != 4)
        return ERROR(void, ERR_TERM_GET_CURSOR_POS);

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[')
        return ERROR(void, ERR_TERM_GET_CURSOR_POS);

    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
        return ERROR(void, ERR_TERM_GET_CURSOR_POS);

    return OK(void);
}

static RESULT(void) terminalGetWindowSize(int *rows, int *cols) {
    if (WRITE_SEQ(MOVE_CURSOR_TO_BOTTOM_RIGHT) != 12)
        return ERROR(void, ERR_TERM_ESC_SEQ);

    return terminalGetCursorPosition(rows, cols);
}
