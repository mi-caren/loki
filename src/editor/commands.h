#ifndef EDITOR_COMMAND_H
#define EDITOR_COMMAND_H

#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>

#include "editing_point.h"
#include "editor/utils.h"
#include "utils/result.h"


typedef enum {
    CCMD_INSERT_CHAR,
    CCMD_DELETE_CHAR,
} CoreCommandType;

/*
 * Represents a basic editing command.
 * Every command that performs an operation on the buffer
 * (inserting / delering a character, adding a new line, pasting a buffer, deleting a selection)
 * can be semplified in a series of basic insertion / deletion of character.
 */
typedef struct {
    /* The type of this basic command (INSERT_CHAR / DELETE_CHAR) */
    CoreCommandType type;
    /* The point in the buffer where the command was executed */
    EditingPoint ep;
    /* The inserted / deleted char */
    char c;
} CoreCommand;

// typedef Command* CommandPtr;

// RESULT_STRUCT_DEF(CommandPtr);

// TRY_FUNC_SIGNATURE(CommandPtr);

// ERROR_FUNC_SIGNATURE(CommandPtr);
// OK_FUNC_SIGNATURE(CommandPtr);


// RESULT(CommandPtr) buildCommand(int key);
// void commandExecute(Command* cmd);

bool cmdQuit();
bool cmdSave();
bool cmdFind();
bool cmdSearchNext();
bool cmdSearchPrev();
bool cmdCopy();
void cmdPaste();
void cmdCut();
bool cmdUndo();
void cmdDelete(bool del_key);

void cmdInsertChar(char c);



#define CMD_ERR_NEW                 ERROR_PARAMS(CmdNew, "Unable to create new editor command")
#define CMD_ERR_NOT_KNOWN           ERROR_PARAMS(CmdNotKnown, "Unknown editor command")


#endif
