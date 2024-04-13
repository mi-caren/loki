#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

/*
 * Reimposta gli attributi del terminale allo stato
 * in cui erano
 */
void disable_raw_mode() {
    // TCSAFLUSH, prima di uscire scarta tutti gli input non letti,
    // quindi non tutto ciò che c'è dopo il carattere 'q' non viene 
    // più passato al terminale ma viene scartato
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

/*
 * Legge gli attributi del terminale,
 * ne modifica alcuni e riscrive gli attributi.
 */
void enable_raw_mode() {
    // Legge gli attributi del terminale nella struct raw
    tcgetattr(STDIN_FILENO, &orig_termios);
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
    raw.c_lflag &= ~(ECHO);
    // scrive il nuovo valore della struct raw
    // TCSAFLUSH specifica quindo devono essere applicate le modifich:
    //    aspetta che tutti gli output siano stati scritti sul terminale
    //    e scarta tutti gli input non letti
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
    enable_raw_mode();

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q');
    return 0;
}
