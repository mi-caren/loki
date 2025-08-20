#ifndef VEC_H
#define VEC_H

#include <stdbool.h>
#include <stdlib.h>

#include "utils.h"

#define VecStructName(TYPE)\
    IF_UNSIGNED(TYPE)(\
        CAT(VEC_, PAREN_CLOSE(TYPE)),\
        IF_STRUCT(TYPE)(\
            CAT(VEC_, PAREN_CLOSE(TYPE)),\
            CAT(Vec, TYPE)\
        )\
    )

#define VEC_unsigned                    VEC_UNSIGNED(
#define VEC_UNSIGNED(TYPE)              PRIMITIVE_CAT(VecUnsigned, TYPE)

#define VEC_struct                      VEC_STRUCT(
#define VEC_STRUCT(TYPE)                PRIMITIVE_CAT(VecStruct, TYPE)

#define VEC_STRUCT_DECL(TYPE)   typedef struct VecStructName(TYPE) VecStructName(TYPE)
#define VEC_STRUCT_DEF(TYPE)\
    typedef struct VecStructName(TYPE) {\
        size_t cap;\
        size_t len;\
        size_t curr;\
        TYPE ptr[];\
    } VecStructName(TYPE)

#define Vec(TYPE)                       VecStructName(TYPE)*

/* ********* vec_new *********** */
#define VEC_NEW_FUNC_NAME(TYPE)         CAT(VecStructName(TYPE), _new)
#define VEC_NEW_FUNC_SIGNATURE(TYPE)    Vec(TYPE) VEC_NEW_FUNC_NAME(TYPE)(size_t initial_size)
#define VEC_NEW_FUNC_IMPL(TYPE)\
    VEC_NEW_FUNC_SIGNATURE(TYPE) {\
        size_t cap = vec_cap_from_size(initial_size);\
\
        Vec(TYPE) vec = malloc(sizeof(VecStructName(TYPE)) + sizeof(TYPE) * cap);\
        if (vec == NULL) return NULL;\
\
        vec->cap = cap;\
        vec->len = 0;\
        vec->curr = 0;\
\
        return vec;\
    }

#define vec_new(TYPE)                   VEC_NEW_FUNC_NAME(TYPE)(1)
#define vec_new_with_cap(TYPE, CAP)     VEC_NEW_FUNC_NAME(TYPE)(CAP)

/* ********* vec_begin *********** */
#define VEC_BEGIN_FUNC_NAME(TYPE)           CAT(VecStructName(TYPE), _begin)
#define VEC_BEGIN_FUNC_SIGNATURE(TYPE)      TYPE* VEC_BEGIN_FUNC_NAME(TYPE)(Vec(TYPE) vec)
#define VEC_BEGIN_FUNC_IMPL(TYPE)\
    VEC_BEGIN_FUNC_SIGNATURE(TYPE) {\
        vec->curr = 0;\
        return vec_curr(TYPE, vec);\
    }

#define vec_begin(TYPE, VEC)                VEC_BEGIN_FUNC_NAME(TYPE)(VEC)

/* ********* vec_curr *********** */
#define VEC_CURR_FUNC_NAME(TYPE)            CAT(VecStructName(TYPE), _curr)
#define VEC_CURR_FUNC_SIGNATURE(TYPE)       TYPE* VEC_CURR_FUNC_NAME(TYPE)(Vec(TYPE) vec)
#define VEC_CURR_FUNC_IMPL(TYPE)\
    VEC_CURR_FUNC_SIGNATURE(TYPE) {\
        if (vec->len == 0) return NULL;\
        if (vec->curr >= vec->len) return NULL;\
        return &vec->ptr[vec->curr];\
    }

#define vec_curr(TYPE, VEC)                 VEC_CURR_FUNC_NAME(TYPE)(VEC)

/* ********* vec_next *********** */
#define VEC_NEXT_FUNC_NAME(TYPE)            CAT(VecStructName(TYPE), _next)
#define VEC_NEXT_FUNC_SIGNATURE(TYPE)       TYPE* VEC_NEXT_FUNC_NAME(TYPE)(Vec(TYPE) vec)
#define VEC_NEXT_FUNC_IMPL(TYPE)\
    VEC_NEXT_FUNC_SIGNATURE(TYPE) {\
        if (vec->curr >= vec->len - 1) return NULL;\
        vec->curr++;\
        return vec_curr(TYPE, vec);\
    }

#define vec_next(TYPE, VEC)                 VEC_NEXT_FUNC_NAME(TYPE)(VEC)

/* ********* vec_empty *********** */
#define VEC_EMPTY_FUNC_NAME(TYPE)            CAT(VecStructName(TYPE), _empty)
#define VEC_EMPTY_FUNC_SIGNATURE(TYPE)       void VEC_EMPTY_FUNC_NAME(TYPE)(Vec(TYPE) vec)
#define VEC_EMPTY_FUNC_IMPL(TYPE)\
    inline VEC_EMPTY_FUNC_SIGNATURE(TYPE) {\
        vec->len = 0;\
    }

#define vec_empty(TYPE, VEC)                 VEC_EMPTY_FUNC_NAME(TYPE)(VEC)

/* ********* vec_push *********** */
#define VEC_PUSH_FUNC_NAME(TYPE)            CAT(VecStructName(TYPE), _push)
#define VEC_PUSH_FUNC_SIGNATURE(TYPE)       Vec(TYPE) VEC_PUSH_FUNC_NAME(TYPE)(Vec(TYPE)* vec_ptr, TYPE el)
#define VEC_PUSH_FUNC_IMPL(TYPE)\
    VEC_PUSH_FUNC_SIGNATURE(TYPE) {\
        if ((*vec_ptr)->len == (*vec_ptr)->cap) {\
            if (vec_grow(TYPE, vec_ptr) == NULL)\
                return NULL;\
        }\
        (*vec_ptr)->ptr[(*vec_ptr)->len] = el;\
        (*vec_ptr)->len++;\
        return *vec_ptr;\
    }

#define vec_push(TYPE, VEC, EL)             VEC_PUSH_FUNC_NAME(TYPE)(&VEC, EL)

/* ********* static vec_grow *********** */
#define VEC_GROW_FUNC_NAME(TYPE)            CAT(VecStructName(TYPE), _grow)
#define VEC_GROW_FUNC_SIGNATURE(TYPE)       Vec(TYPE) VEC_GROW_FUNC_NAME(TYPE)(Vec(TYPE)* vec_ptr)
#define VEC_GROW_FUNC_IMPL(TYPE)\
    static VEC_GROW_FUNC_SIGNATURE(TYPE) {\
        Vec(TYPE) new = realloc(\
            *vec_ptr,\
            sizeof(VecStructName(TYPE)) + sizeof(TYPE) * ((*vec_ptr)->cap) * 2\
        );\
        if (new == NULL) return NULL;\
        *vec_ptr = new;\
        (*vec_ptr)->cap *= 2;\
        return *vec_ptr;\
    }

#define vec_grow(TYPE, VEC_PTR)                 VEC_GROW_FUNC_NAME(TYPE)(VEC_PTR)

/* ********* vec_repeat_append *********** */
#define VEC_REPEAT_APPEND_FUNC_NAME(TYPE)            CAT(VecStructName(TYPE), _repeat_append)
#define VEC_REPEAT_APPEND_FUNC_SIGNATURE(TYPE)       Vec(TYPE) VEC_REPEAT_APPEND_FUNC_NAME(TYPE)(Vec(TYPE)* vec_ptr, TYPE el, size_t n)
#define VEC_REPEAT_APPEND_FUNC_IMPL(TYPE)\
    VEC_REPEAT_APPEND_FUNC_SIGNATURE(TYPE) {\
        size_t total_space = (*vec_ptr)->len + n;\
        if (total_space > (*vec_ptr)->cap) {\
            if (vec_make_space(TYPE, vec_ptr, total_space) == NULL)\
                return NULL;\
        }\
\
        for (size_t i = 0; i < n; i++) {\
            (*vec_ptr)->ptr[(*vec_ptr)->len + i] = el;\
        }\
        (*vec_ptr)->len += n;\
\
        return *vec_ptr;\
    }

#define vec_repeat_append(TYPE, VEC, EL, N)                 VEC_REPEAT_APPEND_FUNC_NAME(TYPE)(&VEC, EL, N)

/* ********* static vec_make_space *********** */
#define VEC_MAKE_SPACE_FUNC_NAME(TYPE)            CAT(VecStructName(TYPE), _make_space)
#define VEC_MAKE_SPACE_FUNC_SIGNATURE(TYPE)       Vec(TYPE) VEC_MAKE_SPACE_FUNC_NAME(TYPE)(Vec(TYPE)* vec_ptr, size_t space)
#define VEC_MAKE_SPACE_FUNC_IMPL(TYPE)\
    static VEC_MAKE_SPACE_FUNC_SIGNATURE(TYPE) {\
        size_t cap = vec_cap_from_size(space);\
        Vec(TYPE) new = realloc(\
            *vec_ptr,\
            sizeof(VecStructName(TYPE)) + sizeof(TYPE) * cap\
        );\
        if (new == NULL) return NULL;\
        *vec_ptr = new;\
        (*vec_ptr)->cap = cap;\
        return *vec_ptr;\
    }

#define vec_make_space(TYPE, VEC_PTR, SPACE)                 VEC_MAKE_SPACE_FUNC_NAME(TYPE)(VEC_PTR, SPACE)


#define VEC_DEFS(TYPE)\
    VEC_STRUCT_DECL(TYPE);\
    VEC_NEW_FUNC_SIGNATURE(TYPE);\
    VEC_BEGIN_FUNC_SIGNATURE(TYPE);\
    VEC_CURR_FUNC_SIGNATURE(TYPE);\
    VEC_NEXT_FUNC_SIGNATURE(TYPE);\
    VEC_EMPTY_FUNC_SIGNATURE(TYPE);\
    VEC_PUSH_FUNC_SIGNATURE(TYPE);\
    VEC_REPEAT_APPEND_FUNC_SIGNATURE(TYPE);

#define VEC_IMPL(TYPE)\
    static VEC_GROW_FUNC_SIGNATURE(TYPE);\
    static VEC_MAKE_SPACE_FUNC_SIGNATURE(TYPE);\
    VEC_STRUCT_DEF(TYPE);\
    VEC_NEW_FUNC_IMPL(TYPE)\
    VEC_BEGIN_FUNC_IMPL(TYPE)\
    VEC_CURR_FUNC_IMPL(TYPE)\
    VEC_NEXT_FUNC_IMPL(TYPE)\
    VEC_EMPTY_FUNC_IMPL(TYPE)\
    VEC_PUSH_FUNC_IMPL(TYPE)\
    VEC_REPEAT_APPEND_FUNC_IMPL(TYPE)\
    VEC_GROW_FUNC_IMPL(TYPE)\
    VEC_MAKE_SPACE_FUNC_IMPL(TYPE)

#define vec_foreach(TYPE, EL, VEC) \
    for (TYPE* EL = vec_begin(TYPE, VEC); EL != NULL; EL = vec_next(TYPE, VEC))

size_t vec_cap_from_size(size_t size);

VEC_DEFS(char)

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