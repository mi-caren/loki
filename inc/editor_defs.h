#ifndef EDITOR_DEFS_H
#define EDITOR_DEFS_H

enum EditorKey {
    BACKSPACE = 127,
    ARROW_LEFT = 1000,    // avoid conflicts with regular chars
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN,

    CTRL_ARROW_UP,
    CTRL_ARROW_DOWN,
    CTRL_ARROW_RIGHT,
    CTRL_ARROW_LEFT,

    SHIFT_ARROW_UP,
    SHIFT_ARROW_DOWN,
    SHIFT_ARROW_RIGHT,
    SHIFT_ARROW_LEFT,

    CTRL_SHIFT_ARROW_RIGHT,
    CTRL_SHIFT_ARROW_LEFT,
};

typedef enum {
    Up,
    Down,
    Left,
    Right,
}  Direction;

/*
 * Print error message and exit with 1
 */
void die(const char *s);

#endif