#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "editor.h"

extern struct Editor editor;
extern struct Terminal terminal;
extern void die(const char *s);



/*
 * Reimposta gli attributi del terminale allo stato
 * in cui erano
 */
void disable_raw_mode() {
    // TCSAFLUSH, prima di uscire scarta tutti gli input non letti,
    // quindi non tutto ciò che c'è dopo il carattere 'q' non viene 
    // più passato al terminale ma viene scartato
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &editor.orig_termios) == -1) {
        die("disable_raw_mode/tcsetattr");
    }
}

/*
 * Legge gli attributi del terminale,
 * ne modifica alcuni e riscrive gli attributi.
 */
void enable_raw_mode() {
    // Legge gli attributi del terminale nella struct raw
    if (tcgetattr(STDIN_FILENO, &editor.orig_termios) == -1) {
        die("enable_raw_mode/tcgetattr");
    }
    // Registriamo una funzione perchè sia chiamata quando
    // il programma termina, o perchè ritorna da main,
    // o perchè viene chiamato exit()
    atexit(disable_raw_mode);
    struct termios raw = editor.orig_termios;
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

RESULT(void) get_cursor_position(int *rows, int *cols) {
    char buf[32];
    unsigned int i = 0;

    RESULT(void) res = INIT_RESULT_VOID;

    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) {
        res.err.code = 1;
        res.err.message = "ERROR terminal/get_cursor_position/request_position";
        return res;
    };

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') {
        res.err.code = 2;
        res.err.message = "ERROR terminal/get_cursor_position/read_escape_seq";
        return res;
    };
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) {
        res.err.code = 3;
        res.err.message = "ERROR terminal/get_cursor_position/read_terminal_response";
        return res;
    };

    return res;
}

RESULT(void) get_window_size(int *rows, int *cols) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) {
        RESULT(void) res = INIT_RESULT_VOID;
        res.err.code = 1;
        res.err.message = "ERROR terminal/get_window_size/move_cursor_to_bottom_right";
        return res;
    };
    return get_cursor_position(rows, cols);
}
