#ifndef MESSAGE_BAR_H
#define MESSAGE_BAR_H

#include <stdbool.h>

#include "utils/dbuf.h"


void messageBarDraw(struct DynamicBuffer *dbuf);
void messageBarSet(const char *fmt, ...);
/*
    Draw a prompt in the message bar and takes an input from the user,
    returning a buffer set with it;
    The space for the user input will be allocated
    by the function. Deallocation is done on by the caller.
    If some error occurs or if the operation is canceled, the buffer is freed by the function and NULL is returned.
    Printing of error informations to the message bar is done by this function.
*/
char* messageBarPrompt(char* prompt, int (*callback)(char*, int));
bool messageBarPromptYesNo(char* prompt);

void infoBarDraw(struct DynamicBuffer *dbuf);


#endif
