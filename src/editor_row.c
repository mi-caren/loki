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

static bool isOperator(char c) {
    return strchr("+-*/%<>=!|&", c) != NULL;
}

const char* C_KEYWORDS[] = {
    "#include", "#define",
    "extern", "return", "sizeof", "typedef",
    "const", "static",
    "switch", "case", "if", "else",
    "for", "do", "while", "continue", "break",
    NULL
};

const char* C_TYPES[] = {
    "char", "int", "float", "double", "bool", "void",
    "signed", "unsigned",
    "short", "long",
    "struct", "enum",
    NULL
};

void editorRowHighlightSyntax(unsigned int filerow) {
    bool in_string = false;
    bool in_comment = false;
    static bool in_multiline_comment = false;
    char opening_string_quote_type = '\0';
    char* single_line_comment_start = "//";
    char* multiline_comment_start = "/*";
    char* multiline_comment_end = "*/";

    struct EditorRow* row = &editor.rows[filerow];

    for (unsigned int i = 0; i < row->size; i++) {
        char c = row->chars[i];

        bool prev_sep = i > 0 ? isSeparator(row->chars[i - 1]) : true;  // beginning of line is considered a separator
        Highlight prev_hl = i > 0 ? row->hl[i - 1] : HL_NORMAL;

        // Highlights strings
        if (in_string) {
            row->hl[i] = HL_STRING;
            if (c == opening_string_quote_type) {
                // if prev char is backsles, closing quote is escaped
                // so it means we are still in string
                if (i > 0 && row->chars[i - 1] == '\\') {
                    // but if the char before \ is not \,
                    // because it would mean we escaped backslash
                    if (i > 1 && row->chars[i - 2] != '\\')
                        continue;
                }

                opening_string_quote_type = '\0';
                in_string = false;
            }
            continue;
        } else {
            if (!(in_comment || in_multiline_comment) && (c == '\'' || c == '"')) {
                row->hl[i] = HL_STRING;
                in_string = true;
                opening_string_quote_type = c;
                continue;
            }
        }

        // Highlights comments
        if (in_comment || in_multiline_comment) {
            if (strncmp(&row->chars[i], multiline_comment_end, strlen(multiline_comment_end)) == 0) {
                for (size_t k = 0; k < strlen(multiline_comment_start); k++) {
                    row->hl[i] = HL_COMMENT;
                    i++;
                }
                i--;
                in_multiline_comment = false;
                continue;
            } else {
                row->hl[i] = HL_COMMENT;
            }
            continue;
        } else {
            if (strncmp(&row->chars[i], single_line_comment_start, strlen(single_line_comment_start)) == 0) {
                row->hl[i] = HL_COMMENT;
                in_comment = true;
                continue;
            } else if (strncmp(&row->chars[i], multiline_comment_start, strlen(multiline_comment_start)) == 0) {
                for (size_t k = 0; k < strlen(multiline_comment_start); k++) {
                    row->hl[i] = HL_COMMENT;
                    i++;
                }
                i--;
                in_multiline_comment = true;
                continue;
            }
        }


        // Highlights numbers
        if ((isdigit(c) && (prev_sep || prev_hl == HL_NUMBER)) || (c == '.' && prev_hl == HL_NUMBER)) {
            row->hl[i] = HL_NUMBER;
            continue;
        }

        // Highlights keywords
        if (prev_sep) {
            int j;
            for (j = 0; C_KEYWORDS[j] != NULL; j++) {
                const char* keyword = C_KEYWORDS[j];
                size_t klen = strlen(keyword);

                if (strncmp(&row->chars[i], keyword, klen) == 0 && isSeparator(row->chars[i + klen])) {
                    for (size_t k = 0; k < klen; k++) {
                        row->hl[i] = HL_KEYWORD;
                        i++;
                    }
                    i--;
                    break;
                }
            }
            if (C_KEYWORDS[j] != NULL) continue;
        }

        // Highlights types
        if (prev_sep) {
            int j;
            for (j = 0; C_TYPES[j] != NULL; j++) {
                const char* type = C_TYPES[j];
                int tlen = strlen(type);

                if (strncmp(&row->chars[i], type, tlen) == 0 && isSeparator(row->chars[i + tlen])) {
                    for (int k = 0; k < tlen; k++) {
                        row->hl[i] = HL_TYPE;
                        i++;
                    }
                    i--;
                    break;
                }
            }
            if (C_TYPES[j] != NULL) continue;
        }

        // Highlights operators
        if (isOperator(c)) {
            row->hl[i] = HL_OPERATOR;
            continue;
        }

    }

    if (filerow == editor.view_rows + editor.rowoff - 1) {
        in_multiline_comment = false;
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
        case HL_NORMAL: return (39 << 8) | 49;  // normal | normal
        case HL_COMMENT: return (90 << 8) | 49; // grey | normal
        case HL_NUMBER: return (95 << 8) | 49;  // bright magenta | normal
        case HL_STRING: return (93 << 8) | 49;  // bright yellow | normal
        case HL_KEYWORD: return (91 << 8) | 49; // bright red | normal
        case HL_TYPE: return (96 << 8) | 49;    // bright cyan | normal
        case HL_OPERATOR: return (91 << 8) | 49;// bright red | normal
        case HL_MATCH: return (39 << 8) | 100;  // normal | grey
        default: return (39 << 8) | 49; // normal | normal
    }
}

int editorRowRender(unsigned int filerow)
{
    struct EditorRow* row = &editor.rows[filerow];
    unsigned int i;
    unsigned int tabs = 0;
    for (i = 0; i < row->size; i++) {
        if (row->chars[i] == '\t') {
            tabs++;
        }
    }

    if (editorRowResetHighlight(row) == -1)
        return -1;
    editorRowHighlightSyntax(filerow);
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
