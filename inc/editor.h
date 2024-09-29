#ifndef EDITOR_H
#define EDITOR_H

#include <termios.h>
#include <stdbool.h>


void editorInit();
int editorOpen(char *filename);
void editorRun();

/*
 * Print error message and exit with 1
 */
void die(const char *s);


#endif
