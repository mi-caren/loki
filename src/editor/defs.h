#ifndef EDITOR_DEFS_H
#define EDITOR_DEFS_H

#include <time.h>
#include <stdbool.h>

#include "editing_point.h"
#include "editor/commands.h"

#include "editor_row.h"     // provides Vec(EditorRow)
#include "utils/vec.h"

#define LOKI_VERSION     "0.0.1"

#define CURR_ROW                vec_get(editor.rows, getRow(editor.editing_point))
#define NEXT_ROW                vec_get(editor.rows, getRow(editor.editing_point) + 1)
#define PREV_ROW                vec_get(editor.rows, getRow(editor.editing_point) - 1)
#define ROW_AT(EDITING_POINT)   vec_get(editor.rows, getRow(EDITING_POINT))

#define CURR_CHAR               CURR_ROW->chars[getCol(editor.editing_point)]
#define CHAR_AT(EDITING_POINT)  vec_get(editor.rows, getRow(EDITING_POINT))->chars[getCol(EDITING_POINT)]

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

typedef Vec(CoreCommand)* Command;
VEC_DEFS(Command)

typedef struct Editor {
    /* Number of rows visible on screen */
    unsigned int view_rows;

    EditingPoint editing_point;
    /* x position of rendered view */
    unsigned int rx;

    Vec(EditorRow)* rows;

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
    Vec(char)* copy_buf;

    /*
     * The history of the executed Editor Commands.
     * Every editor command (like pasting a buffer) is represented
     * by a row in this vector. Every row is an array of CoreCommands.
     * Undoing a command means reverting every CoreCommand contained
     * in a row of this vector.
     */
    Vec(Command)* command_history;
    Command* curr_history_cmd;
} Editor;


#endif