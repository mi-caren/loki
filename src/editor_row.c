#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "editor.h"
#include "editor_row.h"



extern struct Editor editor;



int editorRowRender(struct EditorRow *row)
{
    unsigned int i;
    unsigned int tabs = 0;
    unsigned int digits = 0;
    for (i = 0; i < row->size; i++) {
        if (row->chars[i] == '\t') {
            tabs++;
        } else if (isdigit(row->chars[i])) {
            digits++;
        }
    }

    // eventrully free render if it is not null
    // this makes the munction more general because it can be called
    // also to RE-rende a row
    free(row->render);
    char *new = malloc(row->size + 1 + tabs*(TAB_SPACE_NUM - 1) + digits*10);

    if (new == NULL)
        return -1;

    row->render = new;

    unsigned int j = 0;
    for (i = 0; i < row->size; i++) {
        if (row->chars[i] == '\t') {
            unsigned int tab_i;
            for (tab_i = 0; tab_i < TAB_SPACE_NUM; tab_i++)
                row->render[j++] = ' ';
        } else if (isdigit(row->chars[i])) {
            memcpy(&row->render[j], "\x1b[95m", 5);
            j += 5;
            row->render[j++] = row->chars[i];
            memcpy(&row->render[j], "\x1b[39m", 5);
            j += 5;
        } else {
            row->render[j++] = row->chars[i];
        }
    }

    row->render[j] = '\0';
    row->rsize = j;

    return 0;
}

void editorRowInsertChar(struct EditorRow* row, unsigned int pos, char c)
{
    if (pos > row->size)
        pos = row->size;
    row->chars = realloc(row->chars, row->size + 2);
    memmove(&row->chars[pos + 1], &row->chars[pos], row->size - pos + 1);
    row->size++;
    row->chars[pos] = c;
    editorSetDirty();
    editorRowRender(row);
}

void editorRowDeleteChar(struct EditorRow* row, unsigned int pos)
{
    if (pos >= row->size) return;
    memmove(&row->chars[pos], &row->chars[pos + 1], row->size - pos);
    row->size--;
    editorSetDirty();
    editorRowRender(row);
}

void editorRowFree(struct EditorRow* row)
{
    free(row->chars);
    free(row->render);
}

struct EditorRow* editorRowAppendString(struct EditorRow* row, char* s, size_t len)
{
    char* new = realloc(row->chars, row->size + len + 1);
    if (new == NULL) {
        return NULL; // error
    }
    memcpy(&new[row->size], s, len);
    row->chars = new;
    row->size += len;
    row->chars[row->size] = '\0';
    editorRowRender(row);
    editorSetDirty();

    return row;
}
