#ifndef EDITOR_H
#define EDITOR_H

#include "editor/defs.h"
#include <termios.h>
#include <stdbool.h>


void editorInit(Editor* ed);
int editorOpen(char *filename);
void editorRun();

/*
 * Print error message and exit with 1
 */
void die(const char *s);


#endif
