#include <stdio.h>

#include "utils/dbuf.h"


void editorProcessKeypress();
int editorReadKey();
void editorSetDirty();
void editorExitError(const char *s);

void editorCxToRx();
void editorScroll();
void editorRefreshScreen();

void editorDeleteRow(unsigned int pos);
void editorDrawRow(unsigned int filerow, struct DynamicBuffer* dbuf);
int editorInsertRow(unsigned int pos, char *s, size_t len);
char* editorRowsToString(int* buflen);
void editorDrawRows(struct DynamicBuffer *dbuf);
