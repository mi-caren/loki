#ifndef VEC_H
#define VEC_H

#include <stdbool.h>
#include <stdlib.h>

/*   VEC   */
#define VEC(TYPE)                       TYPE*
#define VEC_NEW(TYPE)                   vecNew(sizeof(TYPE), 0)
#define VEC_NEW_WITH_CAP(TYPE, CAP)     vecNew(sizeof(TYPE), CAP)
#define VEC_REPEAT_APPEND(VEC, EL, N)   vecRepeatAppend((void**)&VEC, &EL, N)
#define VECPUSH(VEC, EL)                vecPush((void**)&VEC, &EL)
#define VEC_INSERT(VEC, INDEX, EL)      vecInsert((Vec*)&VEC, INDEX, &EL)
#define VECPUSH_CONST(TYPE, VEC, EL)\
    {\
        TYPE el = EL;\
        vecPush(VEC, &el);\
    }
/* Removes the last element of the vector and returns a pointer to it, casting the return value
   to (TYPE*). The TYPE passed MUST be the type of the elements that the vector contains.
   This macro can help avoid errors by avoiding you to assign the return value of vecPop
   to a wrong pointer type. */
#define VEC_POP(TYPE, VEC)      (TYPE*)vecPop(VEC)
#define VEC_FREE(TYPE, VEC)     vecFree((TYPE)VEC)
/* TYPE is the type CONTAINED in the vector */
#define VECFOREACH(TYPE, EL, VEC) \
    for (TYPE* EL = vecBegin(VEC); EL != NULL; EL = vecNext(VEC))

#define VEC_FOREACH_REV(TYPE, EL, VEC) \
    for (TYPE* EL = vecEnd(VEC); EL != NULL; EL = vecPrev(VEC))

#define VEC_FOR(IDX, VEC) \
    for (size_t IDX = 0; IDX < vecLen(VEC); IDX++)

typedef void* Vec;

void* vecNew(size_t sizeof_type, size_t initial_size);
void vecFree(Vec vec);

Vec vecRepeatAppend(Vec* vec, void* el, size_t n);
void* vecPush(Vec* vec, void* el);
/* Inserts a new element at the specified position,
   shifting all elements after it to the right */
void* vecInsert(Vec* vec, unsigned int index, void* el);
/* Removes and returns the element at the specified position,
   shifting all elements after it to the left */
void* vecRemove(Vec vec, unsigned int index);
/*
 * Removes the last element of the vector and returns a pointer to it.
 * The element is removed by just decreasing the length of the vector by one,
 * so the returned element remains a valid pointer until a new push
 * operation is performed.
 * If you need to use the returned value of a pop even after making some pushs,
 * remember to copy the content of the memory location referenced by the pop return value:
 *      int* tmp = vecPop(vector);
 *      int x;
 *      memcpy(&x, tmp, sizeof(int));
 *      int newVal = 5;
 *      VECPUSH(vector, newVal);
 *
 * Because vectors must be generic we can only return pointers,
 * and not the actual element because we don't know which type
 * the vector contains.
 * There's the risk of creating error not detected because, by returning a
 * void pointer, that value can be assigned to any pointer.
 * This for example is legal but will probably produce runtime memory errors:
 *      VEC(int) vector = VEC_NEW(int);
 *      char* c = vecPop(vector);
 * It's even easier to make mistakes if the vector contains pointers.
 * The VEC_POP macro can help spotting errors because will cast the return value
 * of vecPop.
 */
void* vecPop(Vec vec);

size_t vecLen(Vec vec);

void vecEmpty(Vec vec);

/* Resets the vector iterator to its first element and returns that element */
void* vecBegin(Vec vec);
/* Sets the vector iterator to its last element and returns that element */
void* vecEnd(Vec vec);

void* vecNext(Vec vec);
void* vecPrev(Vec vec);
void* vecSetAt(Vec vec, size_t pos);

void* vecCurr(Vec vec);
void* vecFirst(Vec vec);
void* vecLast(Vec vec);

void* vecAt(Vec vec, size_t pos);
size_t vecCurrIdx(Vec vec);


#endif