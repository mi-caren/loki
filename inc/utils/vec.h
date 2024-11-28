#ifndef VEC_H
#define VEC_H

#include <bits/types/struct_iovec.h>
#include <stdbool.h>

/*   VEC   */
#define VEC(type) vecNew(sizeof(type))
#define VECPUSH(VEC, EL) vecPush((void**)&VEC, EL)
#define VECPUSH_CONST(TYPE, VEC, EL)\
    {\
        TYPE el = EL;\
        vecPush(VEC, &el);\
    }
#define VECFOREACH(TYPE, EL, VEC)\
    size_t idx = 0;\
    for (TYPE EL = VEC[0]; idx < vecLen(VEC); EL = VEC[++idx])

typedef void* Vec;

void* vecNew(size_t sizeof_type);

void* vecPush(void** vec, void* el);
void* vecPop(Vec vec);

size_t vecLen(void* vec);

void vecEmpty(void* vec);

void* vecBegin(void* vec);
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