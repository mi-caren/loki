#ifndef VEC_H
#define VEC_H

#include <stdbool.h>
#include <stdlib.h>

/*   VEC   */
#define VEC(TYPE)           TYPE*
#define VECNEW(TYPE)        vecNew(sizeof(TYPE))
#define VECPUSH(VEC, EL)    vecPush((void**)&VEC, &EL)
#define VECPUSH_CONST(TYPE, VEC, EL)\
    {\
        TYPE el = EL;\
        vecPush(VEC, &el);\
    }
#define VECFOREACH(TYPE, EL, VEC)\
    for (TYPE* EL = vecBegin(VEC); EL != NULL; EL = vecNext(VEC))

typedef void* Vec;

void* vecNew(size_t sizeof_type);

void* vecPush(Vec* vec, void* el);
void* vecPop(Vec vec);

size_t vecLen(Vec vec);

void vecEmpty(Vec vec);

void* vecBegin(Vec vec);
void* vecEnd(Vec vec);

void* vecNext(Vec vec);
void* vecPrev(Vec vec);
void* vecCurr(Vec vec);
void* vecFirst(Vec vec);
void* vecLast(Vec vec);

void* vecAt(Vec vec, size_t pos);
size_t vecGetCurrIdx(Vec vec);
bool vecSetAt(Vec vec, size_t pos);


#endif