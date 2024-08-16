#ifndef EDITING_POINT_H
#define EDITING_POINT_H

#include "utils.h"

void editingPointMoveToChar(Direction dir);
void editingPointMoveToWord(Direction dir);
void editingPointMoveToParagraph(Direction dir);
void editingPointMove(enum EditorKey key);

#endif
