#ifndef EDITING_POINT_H
#define EDITING_POINT_H

#include "utils.h"

struct EditingPoint {
    unsigned int cx;
    unsigned int cy;
};

void editingPointMove(enum EditorKey key);

#endif
