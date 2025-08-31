#ifndef ARRAY_H
#define ARRAY_H

#include "utils.h"

/* *********** ARRAY ******************** */
#define CAPITAL(TYPE)   CAT(CAPITAL_, TYPE)
#define CAPITAL_int     Int
#define CAPITAL_void    Void

#define ARRAY_NAME(TYPE)            CAT(Array, CAPITAL(TYPE))
#define ARRAY_NAME_UNSIGNED(TYPE)   CAT(ArrayUnsigned, CAPITAL(TYPE))

#define ARRAY_MEMBERS(TYPE)\
    TYPE* ptr;\
    size_t cur;\
    size_t len;\
    size_t cap;

#define DEF_ARRAY(TYPE)\
    typedef struct {\
        ARRAY_MEMBERS(TYPE)\
    } ARRAY_NAME(TYPE)

#define DEF_ARRAY_UNSIGNED(TYPE)\
    typedef struct {\
        ARRAY_MEMBERS(unsigned TYPE)\
    } ARRAY_NAME_UNSIGNED(TYPE)

#define ARRAY_PUSH_SIG(TYPE)\
    ARRAY_NAME(TYPE)* CAT(arrayPush, CAPITAL(TYPE))(ARRAY_NAME(TYPE)* a, TYPE el)
#define DEF_ARRAY_PUSH(TYPE)\
    ARRAY_PUSH_SIG(TYPE) {\
        if (!arrayIncSize((ArrayVoid*)a, sizeof(TYPE))) return NULL;\
        TYPE* ptr = a->ptr;\
        ptr[a->len++] = el;\
        return a;\
    }

#define ARRAY_PUSH_UNSIGNED_SIG(TYPE)\
    ARRAY_NAME_UNSIGNED(TYPE)* CAT(arrayPushUnsigned, CAPITAL(TYPE))(ARRAY_NAME_UNSIGNED(TYPE)* a, unsigned TYPE el)
#define DEF_ARRAY_PUSH_UNSIGNED(TYPE)\
    ARRAY_PUSH_UNSIGNED_SIG(TYPE) {\
        if (!arrayIncSize((ArrayVoid*)a, sizeof(unsigned TYPE))) return NULL;\
        unsigned TYPE* ptr = a->ptr;\
        ptr[a->len++] = el;\
        return a;\
    }
#define ARRAY_START_SIG\
    void arrayStart(ArrayVoid* a)
#define DEF_ARRAY_START\
    ARRAY_START_SIG {\
        a->cur = 0;\
    }
#define ARRAY_START(A)  arrayStart((ArrayVoid*)A)

#define ARRAY_END_SIG\
    void arrayEnd(ArrayVoid* a)
#define DEF_ARRAY_END\
    ARRAY_END_SIG {\
        a->cur = a->len;\
    }
#define ARRAY_END(A)  arrayEnd((ArrayVoid*)A)

#define ARRAY_NEXT_UNSIGNED_SIG(TYPE)\
    unsigned TYPE* CAT(arrayNextUnsigned, CAPITAL(TYPE))(ARRAY_NAME_UNSIGNED(TYPE)* a)
#define DEF_ARRAY_NEXT_UNSIGNED(TYPE)\
    ARRAY_NEXT_UNSIGNED_SIG(TYPE) {\
        if (a->cur == a->len) return NULL;\
        return &a->ptr[a->cur++];\
    }

#define ARRAY_PREV_UNSIGNED_SIG(TYPE)\
    unsigned TYPE* CAT(arrayPrevUnsigned, CAPITAL(TYPE))(ARRAY_NAME_UNSIGNED(TYPE)* a)
#define DEF_ARRAY_PREV_UNSIGNED(TYPE)\
    ARRAY_PREV_UNSIGNED_SIG(TYPE) {\
        if (a->cur == 0) return NULL;\
        return &a->ptr[--a->cur];\
    }

/* A is a reference to an array
   the loop will provide a reference to the current element
   accessible via the variable cur, which is a pointer to it*/
#define ARRAY_FOR_EACH_UINT(A)\
    ARRAY_START(A);\
    unsigned int* cur = NULL;\
    while((cur = arrayNextUnsignedInt(A)) != NULL)

#define ARRAY_FOR_EACH_UINT_REV(A)\
    ARRAY_END(A);\
    unsigned int* cur = NULL;\
    while((cur = arrayPrevUnsignedInt(A)) != NULL)

#define ARRAY_EMPTY_SIG\
    void arrayEmpty(ArrayVoid* a)
#define DEF_ARRAY_EMPTY\
    ARRAY_EMPTY_SIG {\
        a->cur = 0;\
        a->len = 0;\
    }
#define ARRAY_EMPTY(A)  arrayEmpty((ArrayVoid*)A)

#define ARRAY_FREE_SIG\
    void arrayFree(ArrayVoid* a)
#define DEF_ARRAY_FREE\
    ARRAY_FREE_SIG {\
        arrayEmpty(a);\
        free(a->ptr);\
        a->ptr = NULL;\
        a->cap = 0;\
    }
#define ARRAY_FREE(A)   arrayFree((ArrayVoid*)A)

#define ARRAY_NEW(ARRAY)   (ARRAY){ NULL, 0, 0, 0 }

DEF_ARRAY(void);
DEF_ARRAY(int);
DEF_ARRAY_UNSIGNED(int);

ARRAY_PUSH_SIG(int);
ARRAY_PUSH_UNSIGNED_SIG(int);

ARRAY_START_SIG;
ARRAY_END_SIG;
ARRAY_NEXT_UNSIGNED_SIG(int);
ARRAY_PREV_UNSIGNED_SIG(int);

ARRAY_EMPTY_SIG;

ARRAY_FREE_SIG;


#endif