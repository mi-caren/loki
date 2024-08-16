#include "editor.h"
#include "editor_row.h"
#include "utils.h"

extern struct Editor editor;


/* Checks if editing point is at the End Of File */
static inline bool editingPointIsEOF() {
    return editor.editing_point.cy == saturating_sub(editor.numrows, 1)
        && editor.editing_point.cx == CURR_ROW.size;
}

/* Checks if editing point is at Beginning Of File */
static inline bool editingPointIsBOF() {
    return editor.editing_point.cy == 0
        && editor.editing_point.cx == 0;
}

static char editingPointPrevChar() {
    if (editor.editing_point.cx == 0) {
        if (editor.editing_point.cy == 0) {
            return '\0';
        }
        return PREV_ROW.chars[PREV_ROW.size]; // if prev_row is empty it will return the string terminator \0
    }

    return CURR_ROW.chars[editor.editing_point.cx - 1];
}

static inline bool currentRowIsFirstRow() {
    return editor.editing_point.cy == 0;
}

static inline bool currentRowIsLastRow() {
    return editor.editing_point.cy == editor.numrows - 1;
}

static struct EditorRow* editingPointPrevRow() {
    if (currentRowIsFirstRow()) return &CURR_ROW;
    return &PREV_ROW;
}


void editingPointMove(Direction dir) {
    if (editor.numrows == 0) {
        return;
    }

    switch (dir) {
        case Up:
            if (editor.editing_point.cy != 0) {
                editor.editing_point.cy--;
            }
            break;
        case Left:
            if (editor.editing_point.cx != 0) {
                editor.editing_point.cx--;
            } else if (editor.editing_point.cy != 0) {
                editor.editing_point.cy--;
                editor.editing_point.cx = CURR_ROW.size;
            }
            break;
        case Down:
            if (editor.editing_point.cy < saturating_sub(editor.numrows, 1)) {
                editor.editing_point.cy++;
            }
            break;
        case Right:
            if (editor.editing_point.cx < CURR_ROW.size) {
                editor.editing_point.cx++;
            } else if (editor.editing_point.cy < saturating_sub(editor.numrows, 1)) {
                editor.editing_point.cy++;
                editor.editing_point.cx = 0;
            }
            break;
    }

    if (editor.editing_point.cx > CURR_ROW.size) {
        editor.editing_point.cx = CURR_ROW.size;
    }
}

void editingPointMoveToWord(Direction dir) {
    if (dir != Left && dir != Right) return;
    bool (*stopCondition)() = dir == Left ? editingPointIsBOF : editingPointIsEOF;

    while (!stopCondition()) {
        editingPointMove(dir);

        if (
            (editor.editing_point.cx == CURR_ROW.size) // stops at the end of the line
            || (editor.editing_point.cx == 0) // stops at the beginning of the line
            || (CHAR_IS_STOPCHAR(editingPointPrevChar()) && !CHAR_IS_STOPCHAR(CURR_CHAR)) // stops if prev char if a stop char
        ) return;
    }
}

void editingPointMoveToParagraph(Direction dir) {
    if (dir != Up && dir != Down) return;
    bool (*stopCondition)() = dir == Up ? currentRowIsFirstRow : currentRowIsLastRow;

    while (!stopCondition()) {
        editingPointMove(dir);

        if (
            editingPointPrevRow()->size == 0
            && !(CURR_ROW.size == 0)
        ) return;
    }
}
