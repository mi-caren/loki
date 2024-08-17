#ifndef MESSAGE_BAR_H
#define MESSAGE_BAR_H

#include <stdbool.h>

#include "utils.h"


void messageBarDraw(struct DynamicBuffer *dbuf);
void messageBarSet(const char *fmt, ...);
char* messageBarPrompt(char* prompt);
bool messageBarPromptYesNo(char* prompt);

void infoBarDraw(struct DynamicBuffer *dbuf);


#endif
