#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "editor.h"
#include "editor_row.h"



extern struct Editor editor;

unsigned int rditorRowResetSyntax

unsigned int editorRowUpdateSyntax(struct EditorRow* row) {
    row->hl = realloc(row->hl, row->size*sizeof(Highlight));

    Highlight prev_hl = -1;
    unsigned int hl_escape_seq_size = 0;

    for (unsigned int i = 0; i < row->size; i++) {
        if (row->hl[i] != HL_MATCH) {
            if (isdigit(row->chars[i])) {
                row->hl[i] = HL_NUMBER;
            } else {
                row->hl[i] = HL_NORMAL;
            }
        }

        if (prev_hl != row->hl[i]) {
            hl_escape_seq_size += 5;
        }

        prev_hl = row->hl[i];
    }

    return hl_escape_seq_size;
}

int syntaxToColor(Highlight hl) {
    switch (hl) {
        case HL_NORMAL: return 39;
        case HL_NUMBER: return 95;
        case HL_MATCH: return 31;
        default: return 39;
    }
}

int editorRowRender(struct EditorRow *row)
{
    unsigned int i;
    unsigned int tabs = 0;
    for (i = 0; i < row->size; i++) {
        if (row->chars[i] == '\t') {
            tabs++;
        }
    }

    // if (editorRowSyntaxReset(row) == -1)
    //     return -1;
    unsigned int syntax_hl_esc_seq_size = editorRowUpdateSyntax(row);
    // unsigned int search_result_hl_esc_seq_size = editorRowHighlightSearchResults(row);

    // eventrully free render if it is not null
    // this makes the munction more general because it can be called
    // also to RE-rende a row
    free(row->render);
    char *new = malloc(
        row->size + 1
        + tabs*(TAB_SPACE_NUM - 1)
        + syntax_hl_esc_seq_size
        // + search_result_hl_esc_seq_size
    );

    if (new == NULL)
        return -1;

    row->render = new;

    unsigned int j = 0;
    int prev_color = -1;
    for (i = 0; i < row->size; i++) {
        int color = syntaxToColor(row->hl[i]);
        if (color != prev_color) {
            char buf[8];
            int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", color);
            memcpy(&row->render[j], buf, clen);
            j += clen;
        }
        prev_color = color;

        if (row->chars[i] == '\t') {
            unsigned int tab_i;
            for (tab_i = 0; tab_i < TAB_SPACE_NUM; tab_i++)
                row->render[j++] = ' ';
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
    free(row->hl);
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
