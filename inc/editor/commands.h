#ifndef EDITOR_COMMAND_H
#define EDITOR_COMMAND_H

#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>

#include "editing_point.h"
#include "editor/utils.h"
#include "utils/result.h"

#define DEFAULT_CTX \
{\
    .buf = NULL,\
    .restore_buf = NULL,\
    .restore_buf_len = 0,\
    .editing_point = editor.editing_point,\
}

typedef struct CommandContext {
    char* buf;

    char* restore_buf;
    size_t restore_buf_len;

    EditingPoint editing_point;
} CommandContext;

typedef struct Command {
    CommandContext ctx;
    bool (*execute) (CommandContext* ctx);
    bool (*undo) (CommandContext* ctx);
} Command;

typedef Command* CommandPtr;

RESULT_STRUCT_DEF(CommandPtr);

TRY_FUNC_SIGNATURE(CommandPtr);

ERROR_FUNC_SIGNATURE(CommandPtr);
OK_FUNC_SIGNATURE(CommandPtr);


RESULT(CommandPtr) buildCommand(int key);
void commandExecute(Command* cmd);

void editorDeleteChar();
void editorInsertChar(char c);

void editorPaste();
void editorCut();
void editorDelete(bool del_key);

typedef enum {
    CmdOk = OK_CODE,
    CmdNew,
    CmdNotKnown,
} CommandErrorCode;

#define CMD_ERR_NEW                 ERROR_PARAMS(CmdNew, "Unable to create new editor command")
#define CMD_ERR_NOT_KNOWN           ERROR_PARAMS(CmdNotKnown, "Unknown editor command")


#endif
