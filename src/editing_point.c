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


static void editingPointMoveToChar(Direction dir) {
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

static void editingPointMoveToWord(Direction dir) {
    if (dir != Left && dir != Right) return;
    bool (*stopCondition)() = dir == Left ? editingPointIsBOF : editingPointIsEOF;

    while (!stopCondition()) {
        editingPointMoveToChar(dir);

        if (
            (editor.editing_point.cx == CURR_ROW.size) // stops at the end of the line
            || (editor.editing_point.cx == 0) // stops at the beginning of the line
            || (CHAR_IS_STOPCHAR(editingPointPrevChar()) && !CHAR_IS_STOPCHAR(CURR_CHAR)) // stops if prev char if a stop char
        ) return;
    }
}

static void editingPointMoveToParagraph(Direction dir) {
    if (dir != Up && dir != Down) return;
    bool (*stopCondition)() = dir == Up ? currentRowIsFirstRow : currentRowIsLastRow;

    while (!stopCondition()) {
        editingPointMoveToChar(dir);

        if (
            editingPointPrevRow()->size == 0
            && !(CURR_ROW.size == 0)
        ) return;
    }
}

/*
    Takes an EditorKey as parameter and moves the editing point accordingly:

    - HOME_KEY moves the editing point to the beginning of the line
    - END_KEY moves the editing point the the end of the line
    - PAGE_UP and PAGE_DOWN move the editing point up and down by as many row as the screen rows
    - ARROW_UP, ARROW_DOWN, ARROW_LEFT, ARROW_RIGHT move the editing point up, down, left and right by one character
      ensuring the cursor does not exit the rows boundaries
    - CTRL_ARROW_UP and CTRL_ARROW_DOWN move the editing point up and down by one paragraph
    - CTRL_ARROW_LEFT and CTRL_ARROW_RIGHT move the editing point left and right by one word

    Dies if a wrong key is passed.
*/
void editingPointMove(enum EditorKey key) {
    switch (key) {
        case HOME_KEY:
            editor.editing_point.cx = 0;
            break;
        case END_KEY:
            editor.editing_point.cx = CURR_ROW.size;
            break;

        case PAGE_UP:
        case PAGE_DOWN:
            {
                if (key == PAGE_UP) {
                    editor.editing_point.cy = editor.rowoff;
                } else {
                    editor.editing_point.cy = editor.rowoff + editor.view_rows - 1;
                    if (editor.editing_point.cy > editor.numrows)
                        editor.editing_point.cy = editor.numrows;
                }
                int times = editor.view_rows;
                while (times--) {
                    editingPointMoveToChar(key == PAGE_UP ? Up : Down);
                }
            }
            break;

        case ARROW_UP:
        case ARROW_LEFT:
        case ARROW_DOWN:
        case ARROW_RIGHT:
        case SHIFT_ARROW_UP:
        case SHIFT_ARROW_LEFT:
        case SHIFT_ARROW_DOWN:
        case SHIFT_ARROW_RIGHT:
            editingPointMoveToChar(editorKeyToDirection(key));
            break;

        case CTRL_ARROW_UP:
        case CTRL_ARROW_DOWN:
            editingPointMoveToParagraph(editorKeyToDirection(key));
            break;
        case CTRL_ARROW_RIGHT:
        case CTRL_ARROW_LEFT:
        case CTRL_SHIFT_ARROW_RIGHT:
        case CTRL_SHIFT_ARROW_LEFT:
            editingPointMoveToWord(editorKeyToDirection(key));
            break;
        default:
            die("editing_point/editingPointMove");
            break;
    }
}
