#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>

#include "editing_point.h"
#include "editor/utils.h"


typedef struct CommandContext {
    char* buf;

    char* resore_buf;
    size_t restore_buf_len;

    EditingPoint editing_point;
} CommandContext;

typedef struct Command {
    CommandContext ctx;
    bool (*execute) (CommandContext* ctx);
    bool (*undo) (CommandContext* ctx);
} Command;


bool buildCommand(Command* cmd, int key);
void commandExecute(Command* cmd);

void editorDeleteChar();
void editorInsertChar(char c);

void editorPaste();
void editorCut();
void editorDelete(bool del_key);



