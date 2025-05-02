#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "editing_point.h"
#include "editor/defs.h"
#include "editor/utils.h"
#include "utils/string.h"
#include "utils/vec.h"
#include "editor_row.h"

#define PARENTHESIS      "()[]{}"
#define C_OPERATORS     "+-*/%<>=!|&"
#define C_SEPARATORS    ",.+-/*=~%<>;:" C_OPERATORS PARENTHESIS

extern struct Editor editor;


inline EditorRow* editorRowGet(EditingPoint ep) {
    return &editor.rows[getRow(ep)];
}

static void _editorRowResetHighlight(struct EditorRow* row) {
    if (strLen(row->chars) == 0)
        return;

    vecEmpty(row->hl);
    Highlight reset_value = HL_NORMAL;
    VEC_REPEAT_APPEND(row->hl, reset_value, strLen(row->chars));
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

static void _highlightFill(struct EditorRow* row, size_t* i, Highlight val, size_t count) {
    for (size_t k = 0; k < count; k++) {
        row->hl[*i] = val;
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

    struct EditorRow* row = &editor.rows[filerow];

    STR_FOREACH(c, row->chars) {
        size_t i = strCurrIdx(row->chars);

        bool prev_sep = i > 0 ? isSeparator(row->chars[i - 1]) : true;  // beginning of line is considered a separator
        Highlight prev_hl = i > 0 ? row->hl[i - 1] : HL_NORMAL;

        // Highlights includes
        char* include_match = strstr(row->chars, "#include <");
        if (c == '<' && include_match) {
            row->hl[i] = HL_STRING;
            in_string = true;
            continue;
        } else if (c == '>' && include_match) {
            row->hl[i] = HL_STRING;
            in_string = false;
            continue;
        }

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
                _highlightFill(row, &i, HL_COMMENT, strlen(multiline_comment_end));
                strSetAt(row->chars, i);
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
                _highlightFill(row, &i, HL_COMMENT, strlen(multiline_comment_start));
                strSetAt(row->chars, i);
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
                    _highlightFill(row, &i, HL_KEYWORD, klen);
                    strSetAt(row->chars, i);
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
                    _highlightFill(row, &i, HL_TYPE, tlen);
                    strSetAt(row->chars, i);
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

        // Highlights functions
        if (c == '(' && i > 0) {
            row->hl[i] = HL_PARENTHESIS;
            int j = i - 1;
            while (j >= 0 && !isSeparator(row->chars[j])) {
                row->hl[j] = HL_FUNCTION;
                j--;
            }
            if (j != (int)(i - 1)) continue;
        }

        // Highlights parenthesis
        if (isParenthesis(c)) {
            row->hl[i] = HL_PARENTHESIS;
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

void editorRowHighlightSelection(unsigned int filerow) {
    if (!editor.selecting) return;

    static bool in_selection = false;
    struct EditorRow* row = &editor.rows[filerow];

    if (filerow == editor.rowoff && SELECTION_START < editingPointNew(editor.rowoff, 0))
        in_selection = true;

    // I don't use STR_FOR here because I wanto to
    // loop until i <= strLen(row->chars), while
    // STR_FOR loops untile i < strLen(row->chars)
    for (unsigned int i = 0; i <= strLen(row->chars); i++) {
        if (in_selection) {
            if (editingPointNew(filerow, i) == SELECTION_END) {
                in_selection = false;
            } else if (i < strLen(row->chars)) {
                row->hl[i] = HL_SELECTION;
            }
        } else {
            EditingPoint curr_ep = editingPointNew(filerow, i);
            if (curr_ep == SELECTION_START && curr_ep != SELECTION_END) {
                in_selection = true;
                if (i < strLen(row->chars))
                    row->hl[i] = HL_SELECTION;
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
    struct EditorRow* row = &editor.rows[filerow];

    _editorRowResetHighlight(row);
    editorRowHighlightSyntax(filerow);
    if (editor.searching) {
        editorRowHighlightSearchResults(row);
    }
    editorRowHighlightSelection(filerow);

    strEmpty(row->render);

    int prev_color = -1;
    STR_FOREACH(c, row->chars) {
        int color = syntaxToColor(row->hl[vecCurrIdx(row->chars)]);
        int fg = (color >> 8) & 0xff;
        int bg = color & 0xff;
        if (color != prev_color) {
            char buf[16];
            snprintf(buf, sizeof(buf), "\x1b[%03d;%03dm", fg, bg);
            strAppend(&row->render, buf);
        }
        prev_color = color;

        if (c == '\t') {
            strRepeatAppendChar(&row->render, ' ', TAB_SPACE_NUM);
        } else {
            strAppendChar(&row->render, c);
        }
    }

    return 0;
}

void editorRowInsertChar(struct EditorRow* row, unsigned int pos, char c) {
    if (pos > strLen(row->chars))
        pos = strLen(row->chars);

    if (c == '\t') {
        strInsert(&row->chars, pos, "    ");
    } else {
        strInsertChar(&row->chars, pos, c);
    }
}

void editorRowDeleteChar(struct EditorRow* row, unsigned int pos) {
    if (pos >= strLen(row->chars)) return;
    strRemove(row->chars, pos);
    editorSetDirty();
}

void editorRowFree(struct EditorRow* row) {
    strFree(row->chars);
    strFree(row->render);
    vecFree(row->hl);
    ARRAY_FREE(&row->search_match_pos);
}
