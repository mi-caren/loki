#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "aeolus/array.h"
#include "aeolus/result.h"

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
    if (n2 > n1) return 0;
    return n1 -n2;
}

inline unsigned int umin(unsigned int a, unsigned int b) {
    return a < b ? a : b;
}

inline unsigned int umax(unsigned int a, unsigned int b) {
    return a > b ? a : b;
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