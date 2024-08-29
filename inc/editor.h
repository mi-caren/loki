#ifndef EDITOR_H
#define EDITOR_H

#include <termios.h>
#include <time.h>
#include <stdbool.h>

#include "utils.h"


#define CURR_ROW                editor.rows[editor.editing_point.cy]
#define NEXT_ROW                editor.rows[editor.editing_point.cy + 1]
#define PREV_ROW                editor.rows[editor.editing_point.cy - 1]

#define CURR_CHAR               CURR_ROW.chars[editor.editing_point.cx]

#define STATUS_BAR_ROW          ( editor.view_rows + 1 )

#define DEFAULT_EDITING_POINTS_SIZE    128
#define LAST_SEARCH_RESULT             ( editor.search_result.editing_points[editor.search_result.len - 1] )


struct EditingPoint {
    unsigned int cx;
    unsigned int cy;
};

typedef struct {
    char* query;
    struct EditingPoint* editing_points;
    size_t size;
    size_t len;
    size_t curr;
} SearchResult;

struct Editor {
    int view_rows;

    struct EditingPoint editing_point;
    unsigned int rx;

    unsigned int numrows;
    struct EditorRow *rows;

    unsigned int rowoff;
    unsigned int coloff;

    char *filename;

    char message_bar_buf[80];
    time_t message_bar_time;

    bool dirty;

    SearchResult search_result;
    int next_match_result_after_editing_point;
};


int editor_open(char *filename);
void editor_draw_rows(struct DynamicBuffer *dbuf);
void editor_refresh_screen();
void editor_process_keypress();
void editor_run();
void init_editor(int height);
int editor_read_key();
void editorSetDirty();
int editorSearch(char* query);

#endif
