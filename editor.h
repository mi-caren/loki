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
    struct EditorRow *rows;
    int rowoff;
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


RESULT(int) editor_read_key() __attribute__((warn_unused_result));
RESULT(void) editor_open(char *filename) __attribute__((warn_unused_result));
RESULT(void) editor_append_row(char *line, size_t linelen) __attribute__((warn_unused_result));
void editor_draw_rows(struct DynamicBuffer *dbuf);
void editor_refresh_screen();
void editor_move_cursor(int key);
void editor_process_keypress();
RESULT(void) init_editor() __attribute__((warn_unused_result));

#endif
