#ifndef MESSAGE_BAR_H
#define MESSAGE_BAR_H

#include "utils.h"


void messageBarDraw(struct DynamicBuffer *dbuf);
void messageBarSet(const char *fmt, ...);


void infoBarDraw(struct DynamicBuffer *dbuf);


#endif
