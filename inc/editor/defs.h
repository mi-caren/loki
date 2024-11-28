#ifndef EDITOR_DEFS_H
#define EDITOR_DEFS_H

#include <time.h>
#include <stdbool.h>

#include "editing_point.h"
#include "editor/commands.h"

#define LOKI_VERSION     "0.0.1"

#define CURR_ROW                editor.rows[getRow(editor.editing_point)]
#define NEXT_ROW                editor.rows[getRow(editor.editing_point) + 1]
#define PREV_ROW                editor.rows[getRow(editor.editing_point) - 1]

#define CURR_CHAR               CURR_ROW.chars[getCol(editor.editing_point)]
#define CHAR_AT(EDITING_POINT)  editor.rows[getRow(EDITING_POINT)].chars[getCol(EDITING_POINT)]

#define STATUS_BAR_ROW          ( editor.view_rows + 1 )

#define DEFAULT_EDITING_POINTS_SIZE    128
#define LAST_SEARCH_RESULT             ( editor.search_result.editing_points[editor.search_result.len - 1] )

#define SELECTION_START     umin(editor.editing_point, editor.selection_start)
#define SELECTION_END       umax(editor.editing_point, editor.selection_start)


/*
    Stop Chars are those characters that delimits words:
    spaces, null chars.
*/
#define CHAR_IS_STOPCHAR(C)        ( C == ' ' || C == '.' || C == '"' || C == '\'' || C == '(' || C == '[' || C == '{' )

#define COLOR_SEQ_SIZE 10
#define TAB_SPACE_NUM    4


struct Editor {
    int view_rows;

    EditingPoint editing_point;
    unsigned int rx;

    unsigned int numrows;
    struct EditorRow* rows;

    unsigned int rowoff;
    unsigned int coloff;

    char *filename;

    char message_bar_buf[80];
    time_t message_bar_time;

    bool dirty;

    bool searching;
    char* search_query;

    bool selecting;
    EditingPoint selection_start;
    char* copy_buf;

    Command* undoable_command_history;
    Command* curr_cmd;
};


#endif