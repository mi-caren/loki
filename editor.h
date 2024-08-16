#ifndef EDITOR_H
#define EDITOR_H

#include <termios.h>
#include <time.h>
#include <stdbool.h>

#include "utils.h"


#define CURR_ROW                editor.rows[editor.editing_point.cy]
#define NEXT_ROW                editor.rows[editor.editing_point.cy + 1]
#define STATUS_BAR_ROW          ( editor.view_rows + 1 )


struct EditingPoint {
    unsigned int cx;
    unsigned int cy;
};


struct Editor {
    int view_rows;

    struct EditingPoint editing_point;
    unsigned int rx;

    unsigned int numrows;
    struct EditorRow *rows;

    unsigned int rowoff;
    unsigned int coloff;

    char *filename;

    char statusmsg[80];
    time_t statusmsg_time;

    bool dirty;
};

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
};


int editor_open(char *filename);
void editor_draw_rows(struct DynamicBuffer *dbuf);
void editor_refresh_screen();
void editor_move_editing_point(int key);
void editor_process_keypress();
void editor_run();
void init_editor(int height);
void editor_set_status_message(const char *fmt, ...);

#endif
