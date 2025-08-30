#ifndef EDITOR_ROW_H
#define EDITOR_ROW_H

#include <unistd.h>
#include "editing_point.h"
#include "utils/array.h"
#include "utils/string.h"
#include "utils/string.h"
#include "utils/vec.h"


typedef enum {
    HL_NORMAL = 0,
    HL_COMMENT,
    HL_NUMBER,
    HL_STRING,
    HL_KEYWORD,
    HL_TYPE,
    HL_OPERATOR,
    HL_FUNCTION,
    HL_PARENTHESIS,
    HL_MATCH,
    HL_SELECTION,
} Highlight;

VEC_DEFS(Highlight)

typedef struct {
    String chars;
    Vec(Highlight)* hl;

    String render;

    ArrayUnsignedInt search_match_pos;
} EditorRow;

VEC_DEFS(EditorRow)

int editorRowRender(unsigned int filerow);
void editorRowFree(EditorRow* row);
void editorRowInsertChar(EditorRow* row, unsigned int pos, char c);
void editorRowDeleteChar(EditorRow* row, unsigned int pos);

EditorRow* editorRowGet(EditingPoint ep);


#endif
