#ifndef EDITOR_ROW_H
#define EDITOR_ROW_H

#include <unistd.h>

struct EditorRow {
    unsigned int size;
    char *chars;
    unsigned int rsize;
    char *render;
};


int editorRowRender(struct EditorRow *row);
void editorRowFree(struct EditorRow* row);
void editorRowInsertChar(struct EditorRow* row, unsigned int pos, char c);
void editorRowDeleteChar(struct EditorRow* row, unsigned int pos);
struct EditorRow* editorRowAppendString(struct EditorRow* row, char* s, size_t len);



#endif
