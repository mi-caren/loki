#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "aeolus/iterator.h"
#include "editing_point.h"
#include "editor/defs.h"
#include "editor/utils.h"
#include "aeolus/string.h"
#include "aeolus/vec.h"
#include "editor_row.h"
#include "editor/utils.h"

#define PARENTHESIS      "()[]{}"
#define C_OPERATORS     "+-*/%<>=!|&"
#define C_SEPARATORS    ",.+-/*=~%<>;:" C_OPERATORS PARENTHESIS

extern struct Editor editor;


inline EditorRow* editorRowGet(EditingPoint ep) {
    return vec_get(editor.rows, getRow(ep));
}

static void _editorRowResetHighlight(EditorRow* row) {
    if (str_len(&row->chars) == 0)
        return;

    vec_empty(row->hl);
    vec_repeat_append(row->hl, HL_NORMAL, str_len(&row->chars));
}

static bool isSeparator(char c) {
    return isspace(c) || c == '\0' || strchr(C_SEPARATORS, c) != NULL;
}

static bool isOperator(char c) {
    return strchr(C_OPERATORS   , c) != NULL;
}

static bool isParenthesis(char c) {
    return strchr(PARENTHESIS, c);
}

const char* C_KEYWORDS[] = {
    "#include", "#define", "#ifdef", "#ifndef", "#endif",
    "extern", "return", "sizeof", "typedef",
    "const", "static", "inline",
    "switch", "case", "default", "if", "else", "goto",
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

static void _highlightFill(EditorRow* row, size_t* i, Highlight val, size_t count) {
    for (size_t k = 0; k < count; k++) {
        row->hl->items[*i] = val;
        (*i)++;
    }
    (*i)--;
}

void editorRowHighlightSyntax(unsigned int filerow) {
    bool in_string = false;
    bool in_comment = false;
    static bool in_multiline_comment = false;
    char opening_string_quote_type = '\0';
    char* single_line_comment_start = "//";
    char* multiline_comment_start = "/*";
    char* multiline_comment_end = "*/";

    EditorRow* row = vec_get(editor.rows, filerow);
    Highlight* hl = row->hl->items;

    for (EACH(c, &row->chars)) {
        size_t i = str_curri(&row->chars);

        bool prev_sep = i > 0 ? isSeparator(str_char_at(&row->chars, i - 1)) : true;  // beginning of line is considered a separator
        Highlight prev_hl = i > 0 ? hl[i - 1] : HL_NORMAL;

        // Highlights includes
        char* include_match = strstr(str_chars(&row->chars), "#include <");
        if (*c == '<' && include_match) {
            hl[i] = HL_STRING;
            in_string = true;
            continue;
        } else if (*c == '>' && include_match) {
            hl[i] = HL_STRING;
            in_string = false;
            continue;
        }

        // Highlights strings
        if (in_string) {
            hl[i] = HL_STRING;
            if (*c == opening_string_quote_type) {
                // if prev char is backsles, closing quote is escaped
                // so it means we are still in string
                if (i > 0 && str_char_at(&row->chars, i - 1) == '\\') {
                    // but if the char before \ is not \,
                    // because it would mean we escaped backslash
                    if (i > 1 && str_char_at(&row->chars, i - 2) != '\\')
                        continue;
                }

                opening_string_quote_type = '\0';
                in_string = false;
            }
            continue;
        } else {
            if (!(in_comment || in_multiline_comment) && (*c == '\'' || *c == '"')) {
                hl[i] = HL_STRING;
                in_string = true;
                opening_string_quote_type = *c;
                continue;
            }
        }

        // Highlights comments
        if (in_comment || in_multiline_comment) {
            if (strncmp(&str_chars(&row->chars)[i], multiline_comment_end, strlen(multiline_comment_end)) == 0) {
                _highlightFill(row, &i, HL_COMMENT, strlen(multiline_comment_end));
                str_set_at(&row->chars, i);
                in_multiline_comment = false;
                continue;
            } else {
                hl[i] = HL_COMMENT;
            }
            continue;
        } else {
            if (strncmp(&str_chars(&row->chars)[i], single_line_comment_start, strlen(single_line_comment_start)) == 0) {
                hl[i] = HL_COMMENT;
                in_comment = true;
                continue;
            } else if (strncmp(&str_chars(&row->chars)[i], multiline_comment_start, strlen(multiline_comment_start)) == 0) {
                _highlightFill(row, &i, HL_COMMENT, strlen(multiline_comment_start));
                str_set_at(&row->chars, i);
                in_multiline_comment = true;
                continue;
            }
        }


        // Highlights numbers
        if ((isdigit(*c) && (prev_sep || prev_hl == HL_NUMBER)) || (*c == '.' && prev_hl == HL_NUMBER)) {
            hl[i] = HL_NUMBER;
            continue;
        }

        // Highlights keywords
        if (prev_sep) {
            int j;
            for (j = 0; C_KEYWORDS[j] != NULL; j++) {
                const char* keyword = C_KEYWORDS[j];
                size_t klen = strlen(keyword);

                if (strncmp(&str_chars(&row->chars)[i], keyword, klen) == 0 && isSeparator(str_char_at(&row->chars, i + klen))) {
                    _highlightFill(row, &i, HL_KEYWORD, klen);
                    str_set_at(&row->chars, i);
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

                if (strncmp(&str_chars(&row->chars)[i], type, tlen) == 0 && isSeparator(str_char_at(&row->chars, i + tlen))) {
                    _highlightFill(row, &i, HL_TYPE, tlen);
                    str_set_at(&row->chars, i);
                    break;
                }
            }
            if (C_TYPES[j] != NULL) continue;
        }

        // Highlights operators
        if (isOperator(*c)) {
            hl[i] = HL_OPERATOR;
            continue;
        }

        // Highlights functions
        if (*c == '(' && i > 0) {
            hl[i] = HL_PARENTHESIS;
            int j = i - 1;
            while (j >= 0 && !isSeparator(str_char_at(&row->chars, j))) {
                hl[j] = HL_FUNCTION;
                j--;
            }
            if (j != (int)(i - 1)) continue;
        }

        // Highlights parenthesis
        if (isParenthesis(*c)) {
            hl[i] = HL_PARENTHESIS;
            continue;
        }

    }

    if (filerow == editor.view_rows + editor.rowoff - 1) {
        in_multiline_comment = false;
    }
}

void editorRowHighlightSearchResults(EditorRow* row) {
    if (editor.search_query == NULL) return;

    for (EACH(pos, row->search_match_pos)) {
        unsigned int last_pos = *pos + strlen(editor.search_query);
        for (unsigned int j = *pos; j < last_pos; j++) {
            row->hl->items[j] = HL_MATCH;
        }
    }
}

void editorRowHighlightSelection(unsigned int filerow) {
    if (!editor.selecting) return;

    static bool in_selection = false;
    EditorRow* row = vec_get(editor.rows, filerow);
    Highlight* hl = row->hl->items;

    if (filerow == editor.rowoff && SELECTION_START < editingPointNew(editor.rowoff, 0))
        in_selection = true;

    // I don't use STR_FOR here because I wanto to
    // loop until i <= strLen(row->chars), while
    // STR_FOR loops untile i < strLen(row->chars)
    for (unsigned int i = 0; i <= str_len(&row->chars); i++) {
        if (in_selection) {
            if (editingPointNew(filerow, i) == SELECTION_END) {
                in_selection = false;
            } else if (i < str_len(&row->chars)) {
                hl[i] = HL_SELECTION;
            }
        } else {
            EditingPoint curr_ep = editingPointNew(filerow, i);
            if (curr_ep == SELECTION_START && curr_ep != SELECTION_END) {
                in_selection = true;
                if (i < str_len(&row->chars))
                    hl[i] = HL_SELECTION;
            }
        }
    }

    // always reset in_selection when finished rendering visible rows
    if (filerow == editor.view_rows + editor.rowoff - 1) {
        in_selection = false;
    }
}

int syntaxToColor(Highlight hl) {
    switch (hl) {
        case HL_NORMAL: return (39 << 8) | 49;      // normal | normal
        case HL_COMMENT: return (90 << 8) | 49;     // grey | normal
        case HL_NUMBER: return (95 << 8) | 49;      // bright magenta | normal
        case HL_STRING: return (93 << 8) | 49;      // bright yellow | normal
        case HL_KEYWORD: return (91 << 8) | 49;     // bright red | normal
        case HL_TYPE: return (96 << 8) | 49;        // bright cyan | normal
        case HL_OPERATOR: return (91 << 8) | 49;    // bright red | normal
        case HL_FUNCTION: return (32 << 8) | 49;    // bright green | normal
        case HL_PARENTHESIS: return (34 << 8) | 49; // bright blue | normal;
        case HL_MATCH: return (39 << 8) | 100;      // normal | grey
        case HL_SELECTION: return (39 << 8) | 104;     // normal | bright blue;
        default: return (39 << 8) | 49; // normal | normal
    }
}

int editorRowRender(unsigned int filerow) {
    EditorRow* row = vec_get(editor.rows, filerow);

    _editorRowResetHighlight(row);
    editorRowHighlightSyntax(filerow);
    if (editor.searching) {
        editorRowHighlightSearchResults(row);
    }
    editorRowHighlightSelection(filerow);

    str_empty(&row->render);

    int prev_color = -1;
    for (EACH(c, &row->chars)) {
        int color = syntaxToColor(row->hl->items[str_curri(&row->chars)]);
        int fg = (color >> 8) & 0xff;
        int bg = color & 0xff;
        if (color != prev_color) {
            char buf[16];
            snprintf(buf, sizeof(buf), "\x1b[%03d;%03dm", fg, bg);
            str_append(&row->render, buf);
        }
        prev_color = color;

        if (*c == '\t') {
            str_repeat_appendc(&row->render, ' ', TAB_SPACE_NUM);
        } else {
            str_appendc(&row->render, *c);
        }
    }

    return 0;
}

void editorRowInsertChar(EditorRow* row, unsigned int pos, char c) {
    if (pos > str_len(&row->chars))
        pos = str_len(&row->chars);

    if (c == '\t') {
        str_insert(&row->chars, pos, "    ");
    } else {
        str_insertc(&row->chars, pos, c);
    }
}

void editorRowDeleteChar(EditorRow* row, unsigned int pos) {
    if (pos >= str_len(&row->chars)) return;
    str_remove(&row->chars, pos);
    editorSetDirty();
}

void editorRowFree(EditorRow* row) {
    str_free(&row->chars);
    str_free(&row->render);
    vec_free(row->hl);
    vec_free(row->search_match_pos);
}

VEC_IMPL(Highlight)
VEC_IMPL(EditorRow)
