#ifndef EDITOR_COMMAND_H
#define EDITOR_COMMAND_H

#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>

#include "editing_point.h"
#include "editor/utils.h"

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


Command* buildCommand(int key);
void commandExecute(Command* cmd);

void editorDeleteChar();
void editorInsertChar(char c);

void editorPaste();
void editorCut();
void editorDelete(bool del_key);



#endif
