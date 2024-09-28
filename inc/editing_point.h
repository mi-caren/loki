#ifndef EDITING_POINT_H
#define EDITING_POINT_H


#include <stdint.h>

#include "utils.h"

#define EDITING_POINT_BIT_SIZE (32)
#define EDITING_POINT_ROW_BIT_SIZE (24) // max number of rows in a file: ~16 millions
#define EDITING_POINT_COL_BIT_SIZE (8)  // max row length: 256

typedef uint32_t EditingPoint;

void editingPointMove(enum EditorKey key);

/* Get editing point row */
unsigned int getRow(EditingPoint ep);
/* Get editing point column */
unsigned int getCol(EditingPoint ep);

/* Set the editing point row value.
   If the bit size of val is greater than EDITING_POINT_ROW_BIT_SIZE
   the value will be trimmed. */
void setRow(EditingPoint* ep, unsigned int val);
/* Decrements editing point row */
void decRow(EditingPoint* ep);
/* Increments editing point row */
void incRow(EditingPoint* ep);


/* Set the editing point column value.
   If the bit size of val is greater than EDITING_POINT_COL_BIT_SIZE
   the value will be trimmed. */
void setCol(EditingPoint* ep, unsigned int val);
/* Decrements editing point column */
void decCol(EditingPoint* ep);
/* Increments editing point column */
void incCol(EditingPoint* ep);



#endif
