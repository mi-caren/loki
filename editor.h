#ifndef EDITOR_H
#define EDITOR_H


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


#endif
