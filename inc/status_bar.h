#ifndef MESSAGE_BAR_H
#define MESSAGE_BAR_H

#include <stdbool.h>

#include "utils/dbuf.h"


void messageBarDraw(struct DynamicBuffer *dbuf);
void messageBarSet(const char *fmt, ...);
char* messageBarPrompt(char* prompt, int (*callback)(char*, int));
bool messageBarPromptYesNo(char* prompt);

void infoBarDraw(struct DynamicBuffer *dbuf);


#endif
