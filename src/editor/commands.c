#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_SOURCE

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "editor.h"
#include "editing_point.h"
#include "editor/keys.h"
#include "status_bar.h"
#include "editor_row.h"
#include "editor/defs.h"
#include "editor/utils.h"
#include "editor/search.h"
#include "aeolus/iterator.h"
#include "aeolus/result.h"
#include "aeolus/string.h"
#include "aeolus/vec.h"
#include "editor/commands.h"

#define cmd_err(TYPE, CODE) err(TYPE, CODE, command_strerror)


extern struct Editor editor;

VEC_IMPL(CoreCommand)
VEC_IMPL(Command)


static unsigned int _countLeadingSpacesBeforeCol(EditorRow* row, unsigned int col);
static bool _editorDeleteSelection();

static Result(EditingPoint) _coreInsertNewline(EditingPoint ep);
static void _historyPushCmd(Command cmd);

static char* command_strerror(CommandError code) {
    switch (code) {
        case CMD_ERR_INSERT_ROW:
            return "Command insert row";
        case CMD_ERR_DELETE_NO_ROWS:
            return "Command delete no rows";
        case CMD_ERR_DELETE_CHAR_INVALID_EP:
            return "Command delete char invalid Editing Point";
        case CMD_ERR_INSERT_NEWLINE:
            return "Command insert newline";
    }

    return "Unreachable";
}


static Result(EditingPoint) _coreInsertChar(char c, EditingPoint ep) {
    if (editor.rows->len == 0) {
        if (editorInsertRow(0, "") == -1)
            return cmd_err(EditingPoint, CMD_ERR_INSERT_ROW);
    }

    if (c == '\r')
        return _coreInsertNewline(ep);

    editorRowInsertChar(ROW_AT(ep), getCol(ep), c);
    setCol(&ep, getCol(ep) + (c == '\t' ? 4 : 1));

    return ok(EditingPoint, ep);
}

static Result(char) _coreDeleteChar(EditingPoint ep) {
    if (editor.rows->len == 0)
        return cmd_err(char, CMD_ERR_DELETE_NO_ROWS);

    if (getRow(ep) >= editor.rows->len || getCol(ep) > str_len(&ROW_AT(ep)->chars))
        return cmd_err(char, CMD_ERR_DELETE_CHAR_INVALID_EP);

    if (getCol(ep) == str_len(&ROW_AT(ep)->chars)) {
        EditorRow* next_row = ROW_AT(addRows(ep, 1));
        str_appends(&ROW_AT(ep)->chars, &next_row->chars);
        editorSetDirty();
        editorDeleteRow(getRow(ep) + 1);

        return ok(char, '\r');
    }

    char c = CHAR_AT(ep);
    editorRowDeleteChar(ROW_AT(ep), getCol(ep));
    return ok(char, c);
}

static Result(EditingPoint) _coreInsertNewline(EditingPoint ep) {
    // clear selection
    editor.selecting = false;

    // size of new row is the size of the sliced part of the current
    // row beginning at current cursor position and ending at end of current row
    // + the number of spaces of current row, to mantain current indentation
    // when inserting a newline
    EditorRow* row = editorRowGet(ep);
    unsigned int row_slice_size = str_len(&row->chars) - getCol(ep);
    unsigned int leading_spaces_count = _countLeadingSpacesBeforeCol(row, getCol(ep));
    unsigned int new_row_size =  row_slice_size + leading_spaces_count;

    String new_row = str_new_with_cap(new_row_size);
    // if (new_row_chars == NULL)
    //     goto insert_newline_error;

    str_repeat_appendc(&new_row, ' ', leading_spaces_count);
    str_append(&new_row, &str_chars(&row->chars)[getCol(ep)]);

    int res = editorInsertRow(getRow(ep) + 1, str_chars(&new_row));
    str_free(&new_row);

    if (res == -1)
        goto insert_newline_error;

    // fetch row again because calling editorInsertRow
    // calls realloc. The address of row could change
    row = editorRowGet(ep);
    str_truncate(&row->chars, getCol(ep));

    incRow(&ep);
    setCol(&ep, leading_spaces_count);
    return ok(EditingPoint, ep);

insert_newline_error:
    messageBarSet("Unable to insert new row");
    return cmd_err(EditingPoint, CMD_ERR_INSERT_NEWLINE);
}

void cmdInsertChar(char c) {
    EditingPoint ep = unwrap(EditingPoint, _coreInsertChar(c, editor.editing_point));

    /* Rimuovere la logica per aggiustare l'indentazione
       da dentro _coreInsertNewline. Questa funzione non deve fare altro
       che quello che dice il suo nome.
       Qui devo controllare se c == '\r', nel caso sistemare
       l'indentazione e poi aggiungere gli eventuali core_commands
       aggiuntivi nella command history! */

    // Insert command into command_history
    CoreCommand ccmd = {
        .type = CCMD_INSERT_CHAR,
        .ep = editor.editing_point,
        .c = c,
    };
    Command cmd = vec_new(CoreCommand);
    vec_push(cmd, ccmd);
    _historyPushCmd(cmd);

    editor.editing_point = ep;
    editorSetDirty();
}

void cmdPaste() {
    if (editor.copy_buf == NULL) {
        messageBarSet("Nothing to copy. Copy buffer empty!");
        return;
    };

    Command cmd = vec_new(CoreCommand);

    for (EACH(c, editor.copy_buf)) {
        EditingPoint ep = unwrap(EditingPoint, _coreInsertChar(*c, editor.editing_point));

        // Insert every CoreCommand into the Editor Command
        CoreCommand ccmd = {
            .type = CCMD_INSERT_CHAR,
            .ep = editor.editing_point,
            .c = *c,
        };
        vec_push(cmd, ccmd);

        editor.editing_point = ep;
    }

    _historyPushCmd(cmd);
    editorSetDirty();
    editor.selecting = false;
}

void cmdCut() {
    if(cmdCopy()) {
        _editorDeleteSelection();
    }
}


static bool _editorDeleteSelection() {
    if (!editor.selecting)
        return false;

    EditingPoint selection_start = SELECTION_START;
    editor.editing_point = SELECTION_END;
    editingPointMove(ARROW_LEFT);

    Command cmd = vec_new(CoreCommand);

    while (editor.editing_point >= selection_start) {
        char c = catch(char, _coreDeleteChar(editor.editing_point), err) {
            if (err != CMD_ERR_DELETE_NO_ROWS)
                die("delete selection");
        }

        CoreCommand ccmd = {
            .type = CCMD_DELETE_CHAR,
            .ep = editor.editing_point,
            .c = c,
        };
        vec_push(cmd, ccmd);
        editingPointMove(ARROW_LEFT);
    }
    editingPointMove(ARROW_RIGHT);
    _historyPushCmd(cmd);

    editor.selecting = false;

    return true;
}

void cmdDelete(bool del_key) {
    if (!_editorDeleteSelection()) {
        if (!del_key)
            editingPointMove(ARROW_LEFT);

        char c = catch(char, _coreDeleteChar(editor.editing_point), err) {
            if (err != CMD_ERR_DELETE_NO_ROWS)
                die("cmd delete");
        }

        CoreCommand ccmd = {
            .type = CCMD_DELETE_CHAR,
            .ep = editor.editing_point,
            .c = c,
        };
        Command cmd = vec_new(CoreCommand);
        vec_push(cmd, ccmd);
        _historyPushCmd(cmd);
    }
}

static void _historyFreeTrailingCmds() {
    while (editor.curr_history_cmd != vec_last(editor.command_history)) {
        Command* cmd = vec_pop(editor.command_history);
        vec_free(*cmd);
    }
}

static void _historyPushCmd(Command cmd) {
    assert(cmd != NULL);
    _historyFreeTrailingCmds();
    vec_push(editor.command_history, cmd);
    editor.curr_history_cmd = iter_end(editor.command_history);
}

bool cmdUndo() {
    if (!editor.curr_history_cmd)
        return false;

    // editor.curr_history_cmd is a pointer to an element
    // inside command_history, which is a
    // collection of pointers to commands.
    // we have to dereference to obtain the pointed command.
    Command* cmd = editor.curr_history_cmd;

    for (EACH_REV(ccmd, *cmd)) {
        switch (ccmd->type) {
            case CCMD_INSERT_CHAR:
                unwrap(char, _coreDeleteChar(ccmd->ep));
                break;
            case CCMD_DELETE_CHAR:
                unwrap(EditingPoint, _coreInsertChar(ccmd->c, ccmd->ep));
                break;
        }
    }
    CoreCommand* ccmd = vec_first(*cmd);
    editor.editing_point = ccmd->ep;

    editor.curr_history_cmd = iter_prev(editor.command_history);
    editorSetDirty();
    return true;
}




// ----------------------------------------------
// -------------- STATIC FUNCTIONS --------------
// ----------------------------------------------





static unsigned int _countLeadingSpacesBeforeCol(EditorRow* row, unsigned int col) {
    unsigned int i = 0;
    while (str_char_at(&row->chars, i) == ' ' && i < col) {
        i++;
    }
    return i;
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
bool cmdSave() {
    bool ok = false;

    if (editor.filename == NULL) {
        char* filename = messageBarPrompt("Save as", NULL);
        if (filename == NULL) return false;    // the printing of the error message is handled by messageBarPrompt
        editor.filename = filename;
    }

    String buf = editorRowsToString();
    int len = str_len(&buf);
    // if (buf == NULL) goto end;

    int fd = open(editor.filename, O_RDWR | O_CREAT, 0644);
    if (fd == -1) goto clean_buf;
    if (ftruncate(fd, len) == -1) goto clean_fd;
    if (write(fd, str_chars(&buf), len) != len) goto clean_fd;
    ok = true;
    editor.dirty = false;

clean_fd:
    close(fd);
clean_buf:
    str_free(&buf);
// end:
    if (ok) {
        messageBarSet("%d bytes written", len);
    } else {
        messageBarSet("Unable to save buffer! I/O error: %s", strerror(errno));
    }

    return ok;
}

bool cmdQuit() {
    if (editor.dirty) {
        if (messageBarPromptYesNo("File has unsaved changes. Do you want to save before exiting?")) {
            if (!cmdSave()) {
                return false;
            }
        }
    }

    exit(EXIT_SUCCESS);

    return true; // UNREACHABLE
}

bool cmdFind() {
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

inline bool cmdSearchNext() {
    return searchResultNext();
}

inline bool cmdSearchPrev() {
    return searchResultPrev();
}

bool cmdCopy() {
    int err = 0;
    if (editor.selecting) {
        if (editor.copy_buf == NULL) {
            if (!(editor.copy_buf = vec_new(char))) goto copy_error;
        }

        vec_empty(editor.copy_buf);

        EditingPoint ep = SELECTION_START;
        while (ep != SELECTION_END) {
            if (CHAR_AT(ep) == '\0') {
                char c = '\r';
                if (!vec_push(editor.copy_buf, c)) goto copy_error;
            } else {
                if (!vec_push(editor.copy_buf, CHAR_AT(ep))) goto copy_error;
            }

            if (getCol(ep) == str_len(&vec_get(editor.rows, getRow(ep))->chars)) {
                incRow(&ep);
                setCol(&ep, 0);
            } else {
                incCol(&ep);
            }
        }

        messageBarSet("Copied!");
        return true;
    }

copy_error:
    messageBarSet("Unable to copy %d", err);
    return false;
}

