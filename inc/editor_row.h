#ifndef EDITOR_ROW_H
#define EDITOR_ROW_H

#include <unistd.h>
#include "editing_point.h"
#include "utils/array.h"


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

typedef struct EditorRow {
    unsigned int size;
    char *chars;
    Highlight* hl;

    unsigned int rsize;
    char *render;

    ArrayUnsignedInt search_match_pos;
} EditorRow;

int editorRowRender(unsigned int filerow);
void editorRowFree(struct EditorRow* row);
void editorRowInsertChar(struct EditorRow* row, unsigned int pos, char c);
void editorRowDeleteChar(struct EditorRow* row, unsigned int pos);
struct EditorRow* editorRowAppendString(struct EditorRow* row, char* s, size_t len);

EditorRow* editorRowGet(EditingPoint ep);


#endif
