#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>

#include "editor/utils.h"


void editorDeleteChar();
void editorInsertNewline();
void editorInsertChar(char c);

bool editorSave();
bool editorQuit();
void editorCopy();
void editorPaste();
void editorCut();
void editorDelete(bool del_key);
void editorFind();



