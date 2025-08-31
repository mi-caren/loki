#ifndef EDITING_POINT_H
#define EDITING_POINT_H


#include <stdint.h>
#include "editor/keys.h"
#include "aeolus/result.h"


#define EDITING_POINT_BIT_SIZE (32)
#define EDITING_POINT_ROW_BIT_SIZE (24) // max number of rows in a file: ~16 millions
#define EDITING_POINT_COL_BIT_SIZE (8)  // max row length: 256
#define EDITING_POINT_MAX_ROWS (~(~0u << EDITING_POINT_ROW_BIT_SIZE))
#define EDITING_POINT_MAX_COLS (~(~0u << EDITING_POINT_COL_BIT_SIZE))

typedef uint32_t EditingPoint;

void editingPointMove(enum EditorKey key);

EditingPoint editingPointNew(unsigned int row, unsigned int col);

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
/* Adds numrows rows to ep and return the modified editing point */
EditingPoint addRows(EditingPoint ep, unsigned int numrows);


/* Set the editing point column value.
   If the bit size of val is greater than EDITING_POINT_COL_BIT_SIZE
   the value will be trimmed. */
void setCol(EditingPoint* ep, unsigned int val);
/* Decrements editing point column */
void decCol(EditingPoint* ep);
/* Increments editing point column */
void incCol(EditingPoint* ep);

RESULT_DEFS(EditingPoint)

#endif
