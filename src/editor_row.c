#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "editor.h"
#include "utils.h"
#include "editor_row.h"

// #define COLOR_SEQ_SIZE 10
// #define COLOR_SEQ_SIZE 6

extern struct Editor editor;

int editorRowResetHighlight(struct EditorRow* row) {
    if (row->size == 0) return 0;   // realloc sometimes throws double free() error if called with size: 0

    Highlight* new = realloc(row->hl, row->size*sizeof(Highlight));
    if (!new) return -1;
    row->hl = new;
    for (unsigned int i = 0; i < row->size; i++) {
        row->hl[i] = HL_NORMAL;
    }
    return 0;
}

static bool isSeparator(char c) {
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL;
}

void editorRowHighlightSyntax(struct EditorRow* row) {
    bool prev_sep = true;
    Highlight prev_hl = HL_NORMAL;

    for (unsigned int i = 0; i < row->size; i++) {
        char c = row->chars[i];
        if (isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) {
            row->hl[i] = HL_NUMBER;
        }

        prev_sep = isSeparator(c);
        prev_hl = row->hl[i];
    }
}

void editorRowHighlightSearchResults(struct EditorRow* row) {
    if (editor.search_query == NULL) return;

    ARRAY_FOR_EACH_UINT(&row->search_match_pos) {
        unsigned int last_pos = *cur + strlen(editor.search_query);
        for (unsigned int j = *cur; j < last_pos; j++) {
            row->hl[j] = HL_MATCH;
        }
    }
}

int syntaxToColor(Highlight hl) {
    switch (hl) {
        case HL_NORMAL: return (39 << 8) | 49;
        case HL_NUMBER: return (95 << 8) | 49;
        case HL_MATCH: return (39 << 8) | 100;
        default: return (39 << 8) | 49;
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

    if (editorRowResetHighlight(row) == -1)
        return -1;
    editorRowHighlightSyntax(row);
    if (editor.searching) {
        editorRowHighlightSearchResults(row);
    }

    Highlight prev_hl = -1;
    unsigned int hl_escape_seq_size = 0;
    for (i = 0; i < row->size; i++) {
        if (prev_hl != row->hl[i]) {
            hl_escape_seq_size += COLOR_SEQ_SIZE;
        }

        prev_hl = row->hl[i];
    }

    // eventrully free render if it is not null
    // this makes the munction more general because it can be called
    // also to RE-rende a row
    free(row->render);
    row->render = NULL;
    char *new = malloc(
        row->size + 1
        + tabs*(TAB_SPACE_NUM - 1)
        + hl_escape_seq_size
    );

    if (new == NULL)
        return -1;

    row->render = new;

    unsigned int j = 0;
    int prev_color = -1;
    for (i = 0; i < row->size; i++) {
        int color = syntaxToColor(row->hl[i]);
        int fg = (color >> 8) & 0xff;
        int bg = color & 0xff;
        if (color != prev_color) {
            char buf[16];
            int clen = snprintf(buf, sizeof(buf), "\x1b[%03d;%03dm", fg, bg);
            // int clen = snprintf(buf, sizeof(buf), "\x1b[%03dm", fg);
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
}

void editorRowDeleteChar(struct EditorRow* row, unsigned int pos)
{
    if (pos >= row->size) return;
    memmove(&row->chars[pos], &row->chars[pos + 1], row->size - pos);
    row->size--;
    editorSetDirty();
}

void editorRowFree(struct EditorRow* row)
{
    free(row->chars);
    free(row->render);
    free(row->hl);
    ARRAY_FREE(&row->search_match_pos);
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
    editorSetDirty();

    return row;
}
