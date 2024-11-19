#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "editing_point.h"
#include "status_bar.h"
#include "editor_row.h"
#include "editor/defs.h"
#include "editor/utils.h"
#include "editor/search.h"
#include "utils/vec.h"
#include "editor/commands.h"

#define DEFAULT_CTX \
{\
    .editing_point = editor.editing_point,\
    .buf = NULL,\
}


extern struct Editor editor;


static unsigned int _countLeadingSpaces(struct EditorRow* row);
static bool _editorDeleteSelection();

static bool _editorSave();
static bool _editorQuit();
static bool _editorFind();
static bool _editorCopy();
// static bool _editorPaste(CommandContext* ctx);
static bool _editorInsertNewline(CommandContext* ctx);

void editorDeleteChar() {
    if (editor.numrows == 0) return;
    if (getCol(editor.editing_point) == 0 && getRow(editor.editing_point) == 0) return;

    if (getCol(editor.editing_point) > 0) {
        editorRowDeleteChar(&CURR_ROW, getCol(editor.editing_point) - 1);
        decCol(&editor.editing_point);
    } else {
        editingPointMove(ARROW_LEFT);
        if (!editorRowAppendString(&CURR_ROW, NEXT_ROW.chars, NEXT_ROW.size)) {
            messageBarSet("Unable to delete char at %d, %d", getCol(editor.editing_point) - 1, getRow(editor.editing_point));
        }
        editorDeleteRow(getRow(editor.editing_point) + 1);
    }
}

static bool _editorInsertNewline(CommandContext* ctx) {
    if (editor.numrows == 0) {
        if (editorInsertRow(0, "", 0) == -1)
            goto insert_newline_error;
    }

    // clear selection
    editor.selecting = false;

    // size of new row is the size of the sliced part of the current
    // row beginning at current cursor position and ending at end of current row
    // + the number of spaces of current row, to mantain current indentation
    // when inserting a newline
    EditorRow* row = editorRowGet(ctx->editing_point);
    unsigned int row_slice_size = row->size - getCol(ctx->editing_point);
    unsigned int leading_spaces_count = _countLeadingSpaces(row);
    unsigned int new_row_size =  row_slice_size + leading_spaces_count;

    char* new_row_chars = malloc(new_row_size + 1);
    if (!new_row_chars)
        goto insert_newline_error;

    memset(new_row_chars, ' ', leading_spaces_count);
    new_row_chars[leading_spaces_count] = '\0';
    strncat(new_row_chars, &row->chars[getCol(ctx->editing_point)], row_slice_size);

    int res = editorInsertRow(getRow(ctx->editing_point) + 1, new_row_chars, new_row_size);
    free(new_row_chars);

    if (res == -1)
        goto insert_newline_error;

    // fetch row again because calling editorInsertRow
    // calls realloc. The address of row could change
    row = editorRowGet(ctx->editing_point);
    row->chars[getCol(ctx->editing_point)] = '\0';
    row->size = getCol(ctx->editing_point);

    editor.editing_point = ctx->editing_point;
    incRow(&editor.editing_point);
    setCol(&editor.editing_point, leading_spaces_count);
    return true;

insert_newline_error:
    messageBarSet("Unable to insert new row");
    return false;
}


void editorInsertChar(char c) {
    if (editor.numrows == 0) {
        editorInsertRow(editor.numrows, "", 0);
    }
    editorRowInsertChar(&CURR_ROW, getCol(editor.editing_point), c);
    setCol(&editor.editing_point, getCol(editor.editing_point) + (c == '\t' ? 4 : 1));
}

void editorPaste() {
    if (editor.copy_buf == NULL) return;

    VECFOREACH(char, c, editor.copy_buf) {
        if (c == '\0') {
            break;
        } else if (c == '\n') {
            CommandContext ctx = DEFAULT_CTX;
            _editorInsertNewline(&ctx);
        } else {
            editorInsertChar(c);
        }
    }
}

void editorCut() {
    if(_editorCopy()) {
        _editorDeleteSelection();
    }
}

void editorDelete(bool del_key) {
    if (!_editorDeleteSelection()) {
        if (del_key)
            editingPointMove(ARROW_RIGHT);
        editorDeleteChar();
    }
}

void commandExecute(Command* cmd) {
    cmd->execute(&cmd->ctx);
}

bool buildCommand(Command* cmd, int key) {
    *cmd = (Command) {
        .ctx = DEFAULT_CTX,
        .execute = NULL,
        .undo = NULL,
    };

    switch (key) {
        case CTRL_KEY('q'):
            cmd->execute = _editorQuit;
            return true;
        case CTRL_KEY('s'):
            cmd->execute = _editorSave;
            return true;
        case CTRL_KEY('f'):
            cmd->execute = _editorFind;
            return true;
        case CTRL_KEY('n'):
            cmd->execute = searchResultNext;
            return true;
        case CTRL_KEY('p'):
            cmd->execute = searchResultPrev;
            return true;
        case CTRL_KEY('c'):
            cmd->execute = _editorCopy;
            return true;
        // case CTRL_KEY('v'):
        //     cmd->ctx.buf = editor.copy_buf;
        //     cmd->execute = _editorPaste;
        //     editor.selecting = false;
        //     break;

        case '\r':
            cmd->execute = _editorInsertNewline;
            return true;

        default:
            return false;
    }
}




// ----------------------------------------------
// -------------- STATIC FUNCTIONS --------------
// ----------------------------------------------





static unsigned int _countLeadingSpaces(struct EditorRow* row) {
    int i = 0;
    while (row->chars[i] == ' ') {
        i++;
    }
    return i;
}

static bool _editorDeleteSelection() {
    if (!editor.selecting)
        return false;

    editor.editing_point = SELECTION_END;
    EditingPoint selection_start = SELECTION_START;
    while (editor.editing_point != selection_start) {
        editorDeleteChar();
    }

    editor.selecting = false;

    return true;
}

// The normal way to overwrite a file is to pass the O_TRUNC flag to open(),
// which truncates the file completely, making it an empty file,
// before writing the new data into it.
// By truncating the file ourselves to the same length as the data we are planning to write into it,
// we are making the whole overwriting operation a little bit safer in case
// the ftruncate() call succeeds but the write() call fails.
// In that case, the file would still contain most of the data it had before.
// But if the file was truncated completely by the open() call and then the write() failed, you’d end up with all of your data lost.

// More advanced editors will write to a new, temporary file,
// and then rename that file to the actual file the user wants to overwrite,
// and they’ll carefully check for errors through the whole process.
/*
    Returns a bool indicating if saving of file has been successfull.
*/
static bool _editorSave() {
    bool ok = false;

    if (editor.filename == NULL) {
        char* filename = messageBarPrompt("Save as", NULL);
        if (filename == NULL) return false;    // the printing of the error message is handled by messageBarPrompt
        editor.filename = filename;
    }

    int len;
    char* buf = editorRowsToString(&len);
    if (buf == NULL) goto end;

    int fd = open(editor.filename, O_RDWR | O_CREAT, 0644);
    if (fd == -1) goto clean_buf;
    if (ftruncate(fd, len) == -1) goto clean_fd;
    if (write(fd, buf, len) != len) goto clean_fd;
    ok = true;
    editor.dirty = false;

clean_fd:
    close(fd);
clean_buf:
    free(buf);
end:
    if (ok) {
        messageBarSet("%d bytes written", len);
    } else {
        messageBarSet("Unable to save buffer! I/O error: %s", strerror(errno));
    }

    return ok;
}

static bool _editorQuit() {
    if (editor.dirty) {
        if (messageBarPromptYesNo("File has unsaved changes. Do you want to save before exiting?")) {
            if (!_editorSave()) {
                return false;
            }
        }
    }

    editorCleanExit();

    return true; // UNREACHABLE
}

static bool _editorFind() {
    EditingPoint prev_editing_point = editor.editing_point;
    unsigned int prev_coloff = editor.coloff;
    unsigned int prev_rowoff = editor.rowoff;
    char* prev_query = editor.search_query;
    editor.searching = true;

    if (messageBarPrompt("Search", editorFindCallback)) {
        free(prev_query);
        return true;
    }

    editor.editing_point = prev_editing_point;
    editor.coloff = prev_coloff;
    editor.rowoff = prev_rowoff;
    editor.search_query = prev_query;
    editor.searching = false;

    return false;
}

static bool _editorCopy() {
    int err = 0;
    if (editor.selecting) {
        if (editor.copy_buf == NULL) {
            if (!(editor.copy_buf = VEC(char))) goto copy_error;
        }

        vecEmpty(editor.copy_buf);

        EditingPoint ep = SELECTION_START;
        while (ep != SELECTION_END) {
            if (CHAR_AT(ep) == '\0') {
                char c = '\n';
                if (!VECPUSH(editor.copy_buf, c)) goto copy_error;
            } else {
                if (!VECPUSH(editor.copy_buf, CHAR_AT(ep))) goto copy_error;
            }

            if (getCol(ep) == editor.rows[getRow(ep)].size) {
                incRow(&ep);
                setCol(&ep, 0);
            } else {
                incCol(&ep);
            }
        }

        char c = '\0';
        if (!VECPUSH(editor.copy_buf, c)) goto copy_error;
        messageBarSet("Copied!");
        return true;
    }

copy_error:
    messageBarSet("Unable to copy %d", err);
    return false;
}

// static bool _editorPaste(CommandContext* ctx) {
//     if (ctx->buf == NULL) return false;

//     VECFOREACH(char, c, editor.copy_buf) {
//         if (c == '\0') {
//             break;
//         } else if (c == '\n') {
//             editorInsertNewline(ctx);
//         } else {
//             editorInsertChar(c);
//         }
//     }

//     return true;
// }
