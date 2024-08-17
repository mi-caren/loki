#ifndef MESSAGE_BAR_H
#define MESSAGE_BAR_H

#include "utils.h"


typedef enum {
    AnswerYes,
    AnswerNo,
} AnswerYN;


void messageBarDraw(struct DynamicBuffer *dbuf);
void messageBarSet(const char *fmt, ...);
char* messageBarPrompt(char* prompt);

void infoBarDraw(struct DynamicBuffer *dbuf);


#endif
