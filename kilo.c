// *** includes ***
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

// *** defines ***
#define CTRL_KEY(k)    ((k) & 0x1f)

// *** data ***
struct termios orig_termios;


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
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        die("disable_raw_mode/tcsetattr");
    }
}

/*
 * Legge gli attributi del terminale,
 * ne modifica alcuni e riscrive gli attributi.
 */
void enable_raw_mode() {
    // Legge gli attributi del terminale nella struct raw
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        die("enable_raw_mode/tcgetattr");
    }
    // Registriamo una funzione perchè sia chiamata quando
    // il programma termina, o perchè ritorna da main,
    // o perchè viene chiamato exit()
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
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

// *** output ***
void editor_refresh_screen() {
    // clear entire screen
    write(STDOUT_FILENO, "\x1b[2J", 4);
    // ensure cursor is positioned top-left
    write(STDOUT_FILENO, "\x1b[H", 3);
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
int main() {
    enable_raw_mode();

    while (1) {
        editor_refresh_screen();
        editor_process_keypress();
    }
    return 0;
}
