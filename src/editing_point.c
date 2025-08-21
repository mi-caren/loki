// used in RESULT_IMPL macro
#include <stdio.h>

#include "editing_point.h"
#include "editor.h"
#include "editor/defs.h"
#include "editor_row.h"
#include "utils/result.h"
#include "utils/string.h"

extern struct Editor editor;

RESULT_IMPL(EditingPoint)

/* Checks if editing point is at the End Of File */
static inline bool editingPointIsEOF() {
    return getRow(editor.editing_point) == saturating_sub(vecLen(editor.rows), 1)
        && getCol(editor.editing_point) == strLen(CURR_ROW.chars);
}

/* Checks if editing point is at Beginning Of File */
static inline bool editingPointIsBOF() {
    return getRow(editor.editing_point) == 0
        && getCol(editor.editing_point) == 0;
}

static char editingPointPrevChar() {
    if (getCol(editor.editing_point) == 0)
        return '\0';

    return CURR_ROW.chars[getCol(editor.editing_point) - 1];
}

static inline bool currentRowIsFirstRow() {
    return getRow(editor.editing_point) == 0;
}

static inline bool currentRowIsLastRow() {
    return getRow(editor.editing_point) == vecLen(editor.rows) - 1;
}

static EditorRow* editingPointPrevRow() {
    if (currentRowIsFirstRow()) return &CURR_ROW;
    return &PREV_ROW;
}


static void editingPointMoveToChar(Direction dir) {
    if (vecLen(editor.rows) == 0) {
        return;
    }

    switch (dir) {
        case Up:
            if (getRow(editor.editing_point) != 0) {
                decRow(&editor.editing_point);
            }
            break;
        case Left:
            if (getCol(editor.editing_point) != 0) {
                decCol(&editor.editing_point);
            } else if (getRow(editor.editing_point) != 0) {
                decRow(&editor.editing_point);
                setCol(&editor.editing_point, strLen(CURR_ROW.chars));
            }
            break;
        case Down:
            if (getRow(editor.editing_point) < saturating_sub(vecLen(editor.rows), 1)) {
                incRow(&editor.editing_point);
            }
            break;
        case Right:
            if (getCol(editor.editing_point) < strLen(CURR_ROW.chars)) {
                incCol(&editor.editing_point);
            } else if (getRow(editor.editing_point) < saturating_sub(vecLen(editor.rows), 1)) {
                incRow(&editor.editing_point);
                setCol(&editor.editing_point, 0);
            }
            break;
        default:
            return;
    }

    if (getCol(editor.editing_point) > strLen(CURR_ROW.chars)) {
        setCol(&editor.editing_point, strLen(CURR_ROW.chars));
    }
}

static void editingPointMoveToWord(Direction dir) {
    if (dir != Left && dir != Right) return;
    bool (*stopCondition)() = dir == Left ? editingPointIsBOF : editingPointIsEOF;

    while (!stopCondition()) {
        editingPointMoveToChar(dir);

        if (
            (getCol(editor.editing_point) == strLen(CURR_ROW.chars)) // stops at the end of the line
            || (getCol(editor.editing_point) == 0) // stops at the beginning of the line
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
            strLen(editingPointPrevRow()->chars) == 0
            && !(strLen(CURR_ROW.chars) == 0)
        ) return;
    }
}

static Direction editorKeyToDirection(enum EditorKey key) {
    switch (key) {
        case CTRL_ARROW_UP:
        case ARROW_UP:
        case SHIFT_ARROW_UP:
            return Up;
        case CTRL_ARROW_DOWN:
        case ARROW_DOWN:
        case SHIFT_ARROW_DOWN:
            return Down;
        case CTRL_ARROW_LEFT:
        case ARROW_LEFT:
        case SHIFT_ARROW_LEFT:
        case CTRL_SHIFT_ARROW_LEFT:
            return Left;
        case CTRL_ARROW_RIGHT:
        case ARROW_RIGHT:
        case SHIFT_ARROW_RIGHT:
        case CTRL_SHIFT_ARROW_RIGHT:
            return Right;
        default:
            return -1;
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
            setCol(&editor.editing_point, 0);
            break;
        case END_KEY:
            setCol(&editor.editing_point, strLen(CURR_ROW.chars));
            break;

        case PAGE_UP:
        case PAGE_DOWN:
            {
                if (key == PAGE_UP) {
                    setRow(&editor.editing_point, editor.rowoff);
                } else {
                    setRow(&editor.editing_point, editor.rowoff + editor.view_rows - 1);
                    if (getRow(editor.editing_point) > vecLen(editor.rows))
                        setRow(&editor.editing_point, vecLen(editor.rows));
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

EditingPoint editingPointNew(unsigned int row, unsigned int col) {
    EditingPoint ep;
    setRow(&ep, row);
    setCol(&ep, col);
    return ep;
}

inline unsigned int getRow(EditingPoint ep) {
    return ep >> EDITING_POINT_COL_BIT_SIZE;
}

inline unsigned int getCol(EditingPoint ep) {
    return ep & ~(~0u << EDITING_POINT_COL_BIT_SIZE);
}

inline void setRow(EditingPoint* ep, unsigned int val) {
    *ep &= ~(~0u << EDITING_POINT_COL_BIT_SIZE);
    *ep |= (val & ~(~0u << EDITING_POINT_ROW_BIT_SIZE)) << EDITING_POINT_COL_BIT_SIZE;
}

inline void decRow(EditingPoint *ep) {
    setRow(ep, getRow(*ep) - 1);
}

inline void incRow(EditingPoint* ep) {
    setRow(ep, getRow(*ep) + 1);
}

inline EditingPoint addRows(EditingPoint ep, unsigned int numrows) {
    setRow(&ep, getRow(ep) + numrows);
    return ep;
}


inline void setCol(EditingPoint* ep, unsigned int val) {
    *ep &= (~0u << EDITING_POINT_COL_BIT_SIZE);
    *ep |= val & ~(~0u << EDITING_POINT_COL_BIT_SIZE);
}

inline void decCol(EditingPoint* ep) {
    setCol(ep, getCol(*ep) - 1);
}

inline void incCol(EditingPoint* ep) {
    setCol(ep, getCol(*ep) + 1);
}
