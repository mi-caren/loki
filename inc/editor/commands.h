#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>

#include "editor/utils.h"


typedef struct CommandContext {
    int tmp;
} CommandContext;

typedef struct Command {
    CommandContext ctx;
    bool (*execute) (CommandContext* ctx);
    void (*undo) (CommandContext* ctx);
} Command;


bool buildCommand(Command* cmd, int key);
void commandExecute(Command* cmd);

void editorDeleteChar();
void editorInsertNewline();
void editorInsertChar(char c);

void editorCopy();
void editorPaste();
void editorCut();
void editorDelete(bool del_key);



