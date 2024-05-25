#ifndef TERMINAL_H
#define TERMINAL_H


#define WRITE_SEQ(SEQ)                          write(STDOUT_FILENO, SEQ##_SEQ, SEQ##_SEQ_SIZE)


#define CLEAR_SCREEN_SEQ                        "\x1b[2J"
#define CLEAR_SCREEN_SEQ_SIZE                   4

#define MOVE_CURSOR_TO_ORIG_SEQ                 "\x1b[H"
#define MOVE_CURSOR_TO_ORIG_SEQ_SIZE            3
#define MOVE_CURSOR_TO_BOTTOM_RIGHT_SEQ         "\x1b[999C\x1b[999B"
#define MOVE_CURSOR_TO_BOTTOM_RIGHT_SEQ_SIZE    12
#define MOVE_CURSOR_TO_SEQ_FMT                  "\x1b[%d;%dH"

#define CLEAR_LINE_CURSOR_TO_END_SEQ            "\x1b[K"
#define CLEAR_LINE_CURSOR_TO_END_SEQ_SIZE       3

#define HIDE_CURSOR_SEQ                         "\x1b[?25l"
#define HIDE_CURSOR_SEQ_SIZE                    6

#define SHOW_CURSOR_SEQ                         "\x1b[?25h"
#define SHOW_CURSOR_SEQ_SIZE                    6

#define REQUEST_CURSOR_POSITION_SEQ             "\x1b[6n"
#define REQUEST_CURSOR_POSITION_SEQ_SIZE        4

#define INVERTED_COLOR_SEQ                      "\x1b[7m"
#define INVERTED_COLOR_SEQ_SIZE                 4

#define NORMAL_FORMATTING_SEQ                   "\x1b[m"
#define NORMAL_FORMATTING_SEQ_SIZE              3


struct CursorPosition {
    unsigned int cx;
    unsigned int cy;
};

struct Terminal {
    struct CursorPosition cursor_pos;
    unsigned int screenrows;
    unsigned int screencols;
};



void disable_raw_mode();
int enable_raw_mode(struct termios *orig_termios);
RESULT(void) get_cursor_position(unsigned int *rows, unsigned int *cols) __attribute__((warn_unused_result));
RESULT(void) get_window_size(unsigned int *rows, unsigned int *cols) __attribute__((warn_unused_result));


#endif
