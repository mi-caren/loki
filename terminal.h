#ifndef TERMINAL_H
#define TERMINAL_H



#define CLEAR_SCREEN_SEQ                "\x1b[2J"
#define CLEAR_SCREEN_SEQ_SIZE           4

#define MOVE_CURSOR_TO_ORIG_SEQ         "\x1b[H"
#define MOVE_CURSOR_TO_ORIG_SEQ_SIZE        3



struct Terminal {
    int cx, cy;
    int screenrows;
    int screencols;
};


void disable_raw_mode();
RESULT(void) enable_raw_mode() __attribute__((warn_unused_result));
RESULT(void) get_cursor_position(int *rows, int *cols) __attribute__((warn_unused_result));
RESULT(void) get_window_size(int *rows, int *cols) __attribute__((warn_unused_result));


#endif
