#include "utils/dbuf.h"
#include "utils/string.h"


void editorProcessKeypress();
int editorReadKey();
void editorSetDirty();
void editorExitError(const char *s);

void editorCxToRx();
void editorScroll();
void editorRefreshScreen();

void editorDeleteRow(unsigned int pos);
void editorDrawRow(unsigned int filerow, struct DynamicBuffer* dbuf);
/* s MUST be 0 terminated */
int editorInsertRow(unsigned int pos, char *s);
String editorRowsToString();
void editorDrawRows(struct DynamicBuffer *dbuf);
