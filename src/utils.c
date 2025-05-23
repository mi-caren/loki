#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils/array.h"
// #include "utils/dbuf.h"
#include "utils/result.h"
#include "utils/vec.h"

static Error __try_error__ = OK_CODE;

RESULT_IMPL(void)
RESULT_IMPL(int)
RESULT_IMPL(unsigned int)
RESULT_IMPL(char)


// void dbuf_append(struct DynamicBuffer *dbuf, const char *s, int len) {
//     char *new = realloc(dbuf->b, dbuf->len + len);

//     if (new == NULL) {
//         die("dbuf_append/realloc");
//     }
//     memcpy(&new[dbuf->len], s, len);
//     dbuf->b = new;
//     dbuf->len += len;
// }

// void dbuf_free(struct DynamicBuffer *dbuf) {
//     free(dbuf->b);
// }


static ArrayVoid* arrayIncSize(ArrayVoid* a, unsigned long type_size) {
    if (a->ptr == NULL) {
        void* new = malloc(type_size);
        if (new == NULL) return NULL;

        a->ptr = new;
        a->cur = 0;
        a->len = 0;
        a->cap = 1;
    }

    if (a->len == a->cap) {
        void* new = realloc(a->ptr, type_size*a->cap*2);
        if (new == NULL) return NULL;

        a->ptr = new;
        a->cap *= 2;
    }

    return a->ptr;
}

DEF_ARRAY_PUSH(int)
DEF_ARRAY_PUSH_UNSIGNED(int)

DEF_ARRAY_NEXT_UNSIGNED(int)
DEF_ARRAY_PREV_UNSIGNED(int)

DEF_ARRAY_START
DEF_ARRAY_END

DEF_ARRAY_EMPTY

DEF_ARRAY_FREE


unsigned int saturating_sub(unsigned int n1, unsigned int n2) {
    int sub = n1 - n2;
    if (sub < 0) return 0;
    return (unsigned int)sub;
}

inline unsigned int umin(unsigned int a, unsigned int b) {
    return a < b ? a : b;
}

inline unsigned int umax(unsigned int a, unsigned int b) {
    return a > b ? a : b;
}


/*   VEC   */
#define VECHEAD(VEC)    ((VecHeader*)((char*)VEC - sizeof(VecHeader)))

typedef struct {
    size_t len;
    size_t cap;
    size_t sizeof_type;
    size_t cur;
} VecHeader;


void* vecNew(size_t sizeof_type, size_t initial_size) {
    if (initial_size == 0) {
        initial_size = 1;
    }

    size_t cap = 1;
    while (cap < initial_size) {
        cap *= 2;
    }

    char* buf = (char*)malloc(sizeof(VecHeader) + sizeof_type * cap);
    if (buf == NULL) return NULL;

    VecHeader* header = (VecHeader*)buf;
    header->len = 0;
    header->cap = cap;
    header->sizeof_type = sizeof_type;
    header->cur = 0;

    return buf + sizeof(VecHeader);
}

inline void vecFree(Vec vec) {
    free(VECHEAD(vec));
}

/* Double the capacity of the vector */
static Vec _vecGrow(Vec* vec) {
    size_t sizeof_type = VECHEAD(*vec)->sizeof_type;
    size_t cap = VECHEAD(*vec)->cap;
    char* new = (char*)realloc(
        (void*)VECHEAD(*vec),
        sizeof(VecHeader) + sizeof_type*cap*2
    );
    if (new == NULL)
        return NULL;

    // If we just took a Vec as parameter: void* _vecGrow(Vec vec)
    // the operation below could not be possible
    // and the user would be forced to reassign the variable every time he calls _vecGrow:
    // myvec = _vecGrow(myvec);
    *vec = new + sizeof(VecHeader);
    VECHEAD(*vec)->cap *= 2;

    return *vec;
}

static Vec _vecMakeSpace(Vec* vec, size_t space) {
    size_t cap = VECHEAD(*vec)->cap;
    if (cap >= space)
        return *vec;

    while (cap < space) {
        cap *= 2;
    }

    size_t sizeof_type = VECHEAD(*vec)->sizeof_type;
    char* new = (char*)realloc(
        (void*)VECHEAD(*vec),
        sizeof(VecHeader) + sizeof_type*cap
    );
    if (new == NULL)
        return NULL;

    *vec = new + sizeof(VecHeader);
    VECHEAD(*vec)->cap = cap;

    return *vec;
}

Vec vecRepeatAppend(Vec* vec, void* el, size_t n) {
    size_t len = VECHEAD(*vec)->len;
    size_t total_space = len + n;
    if (total_space > VECHEAD(*vec)->cap) {
        if (_vecMakeSpace(vec, total_space) == NULL)
            return NULL;
    }

    size_t sizeof_type = VECHEAD(*vec)->sizeof_type;
    for (size_t i = 0; i < n; i++) {
        char* dest = (char*)*vec + sizeof_type*(len + i);
        memcpy(
            dest,
            el,
            sizeof_type
        );
    }
    VECHEAD(*vec)->len += n;

    return *vec;
}

/*
 * Push a new element into the vector.
 * The element must be a void* to let this function.
 * work with any type
 */
void* vecPush(Vec* vec, void* el) {
    if (VECHEAD(*vec)->len == VECHEAD(*vec)->cap) {
        // vecPush calls _vecGrow which reallocs the vector to increase its size.
        // For this reason we need a Vec* and not just a Vec.
        // In this way we can modify reference to the vector without needing
        // the user to reassign it.
        if (_vecGrow(vec) == NULL)
            return NULL;
    }

    char* dest = (char*)*vec;
    memcpy(
        dest + VECHEAD(*vec)->len * VECHEAD(*vec)->sizeof_type,
        el,
        VECHEAD(*vec)->sizeof_type
    );
    VECHEAD(*vec)->len++;

    return *vec;
}

void* vecInsert(Vec* vec, unsigned int index, void* el) {
    if (index > VECHEAD(*vec)->len)
        return NULL;

    if (VECHEAD(*vec)->len == VECHEAD(*vec)->cap) {
        if (_vecGrow(vec) == NULL)
            return NULL;
    }

    memmove(
        (char*)*vec + (index + 1) * VECHEAD(*vec)->sizeof_type,
        (char*)*vec + index * VECHEAD(*vec)->sizeof_type,
        VECHEAD(*vec)->sizeof_type * (VECHEAD(*vec)->len - index)
    );
    memcpy(
        (char*)*vec + index * VECHEAD(*vec)->sizeof_type,
        el,
        VECHEAD(*vec)->sizeof_type
    );
    VECHEAD(*vec)->len++;

    return *vec;
}

void* vecRemove(Vec vec, unsigned int index) {
    if (vecAt(vec, index) == NULL)
        return NULL;

    memmove(
        (char*)vec + index * VECHEAD(vec)->sizeof_type,
        (char*)vec + (index + 1) * VECHEAD(vec)->sizeof_type,
        VECHEAD(vec)->sizeof_type * (VECHEAD(vec)->len - index - 1)
    );
    VECHEAD(vec)->len--;

    return vec;
}

void* vecPop(Vec vec) {
    if (vecLen(vec) == 0) return NULL;

    void* el = vecLast(vec);

    VECHEAD(vec)->len--;
    if (VECHEAD(vec)->cur >= vecLen(vec))
        VECHEAD(vec)->cur = vecLen(vec) - 1;

    return el;
}

inline size_t vecLen(void* vec) {
    return VECHEAD(vec)->len;
}

inline void* vecBegin(Vec vec) {
    VECHEAD(vec)->cur = 0;
    return vecCurr(vec);
}

inline void* vecEnd(Vec vec) {
    if (vecLen(vec) == 0) return NULL;
    VECHEAD(vec)->cur = vecLen(vec) - 1;
    return vecCurr(vec);
}

inline void vecEmpty(void* vec) {
    VECHEAD(vec)->len = 0;
}

inline static void* _vecCurrUnchecked(Vec vec) {
    return (char*)vec + VECHEAD(vec)->cur * VECHEAD(vec)->sizeof_type;
}

inline void* vecNext(Vec vec) {
    if (VECHEAD(vec)->cur >= vecLen(vec) - 1) {
        return NULL;
    }

    VECHEAD(vec)->cur++;
    return vecCurr(vec);
}

inline void* vecPrev(Vec vec) {
    if (VECHEAD(vec)->cur == 0) {
        return NULL;
    }

    VECHEAD(vec)->cur--;
    return vecCurr(vec);
}

inline void* vecCurr(Vec vec) {
    if (VECHEAD(vec)->len == 0) return NULL;
    if (VECHEAD(vec)->cur >= vecLen(vec)) return NULL;
    return _vecCurrUnchecked(vec);
}

inline size_t vecCurrIdx(Vec vec) {
    return VECHEAD(vec)->cur;
}

inline void* vecFirst(Vec vec) {
    return vec;
}

inline void* vecLast(Vec vec) {
    if (vecLen(vec) == 0) return NULL;
    return (char*)vec + (vecLen(vec) - 1) * VECHEAD(vec)->sizeof_type;
}

inline void* vecAt(Vec vec, size_t pos) {
    if (pos >= vecLen(vec)) return NULL;
    return (char*)vec + pos * VECHEAD(vec)->sizeof_type;
}

inline void* vecSetAt(Vec vec, size_t pos) {
    if (pos >= VECHEAD(vec)->len)
        return NULL;

    VECHEAD(vec)->cur = pos;
    return _vecCurrUnchecked(vec);
}


static void (*beforePanicFunc) () = NULL;


void atPanic(void (*beforePanicCallback) ()) {
    beforePanicFunc = beforePanicCallback;
}

// Calls a function registered with atPanic,
// print an error message and exits with error.
void panic(char* filename, int linenumber, char* msg) {
    if (beforePanicFunc) {
        beforePanicFunc();
    }

    fprintf(stderr, "Panicked at %s:%d\n", filename, linenumber);
    if (msg) {
        fprintf(stderr, "%s\n", msg);
    }
    exit(EXIT_FAILURE);
}


inline void __set_try_error__(Error err) {
    __try_error__ = err;
}
inline Error __get_try_error__() {
    return __try_error__;
}