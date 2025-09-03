#include "aeolus/vec.h"
#include "editor/defs.h"
#include "editor/utils.h"
#include "editor_row.h"
#include "status_bar.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

extern struct Editor editor;

bool searchResultNext() {
    // impossible to find a match with 0 rows or without a search query
    if (editor.rows->len == 0 || editor.search_query == NULL)
        return false;

    // clear selection in case user was highlighting something
    editor.selecting = false;
    editor.searching = true;
    messageBarSet("Searching (ESC to cancel): %s", editor.search_query);

    unsigned int cy = getRow(editor.editing_point);
    EditorRow* row = vec_get(editor.rows, cy);
    // First search in current line
    for (EACH(pos, row->search_match_pos)) {
        if (*pos > getCol(editor.editing_point)) {
            setCol(&editor.editing_point, *pos);
            setRow(&editor.editing_point, cy);
            return true;
        }
    }

    // Then search from next line to last line
    for (cy++; cy < editor.rows->len; cy++) {
        row = vec_get(editor.rows, cy);
        if (row->search_match_pos->len > 0) {
            setRow(&editor.editing_point, cy);
            setCol(&editor.editing_point, *vec_first(row->search_match_pos));
            return true;
        }
    }

    // Then search from first line to current line
    for (cy = 0; cy <= getRow(editor.editing_point); cy++) {
        row = vec_get(editor.rows, cy);
        if (row->search_match_pos->len > 0) {
            setRow(&editor.editing_point, cy);
            setCol(&editor.editing_point, *vec_first(row->search_match_pos));
            return true;
        }
    }

    return false;
}

bool searchResultPrev() {
    // impossible to find a match with 0 rows or without a search query
    if (editor.rows->len == 0 || editor.search_query == NULL)
        return false;

    // clear selection in case user was highlighting something
    editor.selecting = false;
    editor.searching = true;
    messageBarSet("Searching (ESC to cancel): %s", editor.search_query);

    int cy = getRow(editor.editing_point);
    EditorRow* row = vec_get(editor.rows, cy);
    // First search in current line
    for (EACH(pos, row->search_match_pos)) {
        if (*pos < getCol(editor.editing_point)) {
            setCol(&editor.editing_point, *pos);
            setRow(&editor.editing_point, cy);
            return true;
        }
    }

    // Then search from prev line to first line
    for (cy--; cy >= 0; cy--) {
        row = vec_get(editor.rows, cy);
        if (row->search_match_pos->len > 0) {
            setRow(&editor.editing_point, cy);
            setCol(&editor.editing_point, *vec_last(row->search_match_pos));
            return true;
        }
    }

    // Then search from last line to current line
    for (cy = editor.rows->len - 1; cy >= (int)getRow(editor.editing_point); cy--) {
        row = vec_get(editor.rows, cy);
        if (row->search_match_pos->len > 0) {
            setRow(&editor.editing_point, cy);
            setCol(&editor.editing_point, *vec_last(row->search_match_pos));
            return true;
        }
    }

    return false;
}

int editorSearch(char* query) {
    editor.search_query = query;

    for (unsigned int i = 0; i < editor.rows->len; i++) {
        char* match = NULL;
        int last_pos = 0;
        EditorRow* row = vec_get(editor.rows, i);
        vec_empty(row->search_match_pos);

        while ((match = strstr(&row->chars[last_pos], query)) != NULL) {
            unsigned int match_pos = match - row->chars;
            if (!vec_push(row->search_match_pos, match_pos)) {
                messageBarSet("Unable to push match result");
                return -1;
            }

            last_pos = match_pos + 1;
        }
    }
    // editorRefreshScreen();

    return 0;
}

int editorFindCallback(char* query, int key) {
    if (key == ARROW_RIGHT || key == ARROW_DOWN || key == CTRL_KEY('n')) {
        searchResultNext();
        return 0;
    } else if (key == ARROW_LEFT || key == ARROW_UP || key == CTRL_KEY('p')) {
        searchResultPrev();
        return 0;
    } else if (key == '\x1b' || key == '\r' || iscntrl(key) || key > 127) {
        return 0;
    }

    // c is a character so i need to perform another search
    if (editorSearch(query) == -1) return -1;
    editorRefreshScreen();

    return 0;
}


