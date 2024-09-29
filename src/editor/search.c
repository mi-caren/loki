#include "editor/defs.h"
#include "editor/utils.h"
#include "editor_row.h"
#include "status_bar.h"
#include "utils/array.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

extern struct Editor editor;

void searchResultNext() {
    if (editor.numrows == 0 || editor.search_query == NULL) return;
    editor.searching = true;
    messageBarSet("Searching (ESC to cancel): %s", editor.search_query);

    unsigned int cy = getRow(editor.editing_point);
    struct EditorRow* row = &editor.rows[cy];
    // First search in current line
    ARRAY_FOR_EACH_UINT(&row->search_match_pos) {
        if (*cur > getCol(editor.editing_point)) {
            setCol(&editor.editing_point, *cur);
            setRow(&editor.editing_point, cy);
            return;
        }
    }

    // Then search from next line to last line
    for (cy++; cy < editor.numrows; cy++) {
        row = &editor.rows[cy];
        if (row->search_match_pos.len > 0) {
            setRow(&editor.editing_point, cy);
            setCol(&editor.editing_point, row->search_match_pos.ptr[0]);
            return;
        }
    }

    // Then search from first line to current line
    for (cy = 0; cy <= getRow(editor.editing_point); cy++) {
        row = &editor.rows[cy];
        if (row->search_match_pos.len > 0) {
            setRow(&editor.editing_point, cy);
            setCol(&editor.editing_point, row->search_match_pos.ptr[0]);
            return;
        }
    }
}

void searchResultPrev() {
    if (editor.numrows == 0 || editor.search_query == NULL) return;
    editor.searching = true;
    messageBarSet("Searching (ESC to cancel): %s", editor.search_query);

    int cy = getRow(editor.editing_point);
    struct EditorRow* row = &editor.rows[cy];
    // First search in current line
    ARRAY_FOR_EACH_UINT_REV(&row->search_match_pos) {
        if (*cur < getCol(editor.editing_point)) {
            setCol(&editor.editing_point, *cur);
            setRow(&editor.editing_point, cy);
            return;
        }
    }

    // Then search from prev line to first line
    for (cy--; cy >= 0; cy--) {
        row = &editor.rows[cy];
        if (row->search_match_pos.len > 0) {
            setRow(&editor.editing_point, cy);
            setCol(&editor.editing_point, row->search_match_pos.ptr[row->search_match_pos.len - 1]);
            return;
        }
    }

    // Then search from last line to current line
    for (cy = editor.numrows - 1; cy >= (int)getRow(editor.editing_point); cy--) {
        row = &editor.rows[cy];
        if (row->search_match_pos.len > 0) {
            setRow(&editor.editing_point, cy);
            setCol(&editor.editing_point, row->search_match_pos.ptr[row->search_match_pos.len - 1]);
            return;
        }
    }
}

int editorSearch(char* query) {
    editor.search_query = query;

    for (unsigned int i = 0; i < editor.numrows; i++) {
        char* match = NULL;
        int last_pos = 0;
        struct EditorRow* row = &editor.rows[i];
        ARRAY_EMPTY(&row->search_match_pos);

        while ((match = strstr(&row->chars[last_pos], query)) != NULL) {
            unsigned int match_pos = match - row->chars;
            if (!arrayPushUnsignedInt(&row->search_match_pos, match_pos)) {
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


