#include <termios.h>
#include <unistd.h>

/*
 * Legge gli attributi del terminale,
 * ne modifica alcuni e riscrive gli attributi.
 */
void enable_raw_mode() {
    struct termios raw;
    // Legge gli attributi del terminale nella struct raw
    tcgetattr(STDIN_FILENO, &raw);
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
