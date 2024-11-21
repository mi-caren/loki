#ifndef VEC_H
#define VEC_H

#include <bits/types/struct_iovec.h>

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
void vecEmpty(void* vec);
size_t vecLen(void* vec);
void vecReset(void* vec);
void* vecNext(Vec vec);
void* vecPrev(Vec vec);
void* vecCurr(Vec vec);



#endif