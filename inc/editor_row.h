#ifndef EDITOR_ROW_H
#define EDITOR_ROW_H

#include <unistd.h>

typedef enum {
    HL_NORMAL = 0,
    HL_NUMBER,
} Highlight;

struct EditorRow {
    unsigned int size;
    char *chars;
    Highlight* hl;

    unsigned int rsize;
    char *render;
};

int editorRowRender(struct EditorRow *row);
void editorRowFree(struct EditorRow* row);
void editorRowInsertChar(struct EditorRow* row, unsigned int pos, char c);
void editorRowDeleteChar(struct EditorRow* row, unsigned int pos);
struct EditorRow* editorRowAppendString(struct EditorRow* row, char* s, size_t len);



#endif
