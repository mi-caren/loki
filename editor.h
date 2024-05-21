#ifndef EDITOR_H
#define EDITOR_H

#include <termios.h>

#include "utils.h"


struct EditorRow {
    int size;
    char *chars;
};

struct Editor {
    int numrows;
    struct EditorRow row;
    struct termios orig_termios;
};

enum EditorKey {
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
void editor_draw_rows(struct DynamicBuffer *dbuf);
void editor_refresh_screen();
void editor_move_cursor(int key);
void editor_process_keypress();
void init_editor();

#endif
