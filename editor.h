#ifndef EDITOR_H
#define EDITOR_H

#include <termios.h>

#include "utils.h"


struct erow {
    int size;
    char *chars;
};

struct editor_config {
    int cx, cy;
    int screenrows;
    int screencols;
    int numrows;
    struct erow row;
    struct termios orig_termios;
};

enum editor_key {
    ARROW_LEFT = 1000,    // avoid conflicts with regular chars
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN,
};


int editor_read_key();
void editor_open(char *filename);
void editor_draw_rows(struct abuf *ab);
void editor_refresh_screen();
void editor_move_cursor(int key);
void editor_process_keypress();
void init_editor();

#endif
