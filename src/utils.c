#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"


extern void die_error(Error err);


UNWRAP_FUNC_DEF(void)
UNWRAP_FUNC_DEF(int)
UNWRAP_FUNC_DEF(unsigned int)



void dbuf_append(struct DynamicBuffer *dbuf, const char *s, int len) {
    char *new = realloc(dbuf->b, dbuf->len + len);

    if (new == NULL) {
        die("dbuf_append/realloc");
    }
    memcpy(&new[dbuf->len], s, len);
    dbuf->b = new;
    dbuf->len += len;
}

void dbuf_free(struct DynamicBuffer *dbuf) {
    free(dbuf->b);
}


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

Direction editorKeyToDirection(enum EditorKey key) {
    switch (key) {
        case CTRL_ARROW_UP:
        case ARROW_UP:
        case SHIFT_ARROW_UP:
            return Up;
        case CTRL_ARROW_DOWN:
        case ARROW_DOWN:
        case SHIFT_ARROW_DOWN:
            return Down;
        case CTRL_ARROW_LEFT:
        case ARROW_LEFT:
        case SHIFT_ARROW_LEFT:
        case CTRL_SHIFT_ARROW_LEFT:
            return Left;
        case CTRL_ARROW_RIGHT:
        case ARROW_RIGHT:
        case SHIFT_ARROW_RIGHT:
        case CTRL_SHIFT_ARROW_RIGHT:
            return Right;
        default:
            die("editor/editorKeyToDirection");
            return 0;    // UNREACHABLE
    }
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
    void* cur;
} VecHeader;

typedef struct {
    VecHeader header;
    void* buf;
} Vec;

void* vecNew(size_t sizeof_type) {
    char* buf = (char*)malloc(sizeof(VecHeader) + sizeof_type);

    if (buf == NULL) return NULL;

    VecHeader* header = (VecHeader*)buf;
    header->len = 0;
    header->cap = 1;
    header->sizeof_type = sizeof_type;
    header->cur = NULL;

    return buf + sizeof(VecHeader);
}

void* vecPush(void** vec, void* el) {
    if (VECHEAD(*vec)->len == VECHEAD(*vec)->cap) {
        size_t sizeof_type = VECHEAD(*vec)->sizeof_type;
        size_t cap = VECHEAD(*vec)->cap;
        char* new = (char*)realloc((void*)VECHEAD(*vec), sizeof(VecHeader) + sizeof_type*cap*2);
        if (new == NULL) return NULL;

        *vec = new + sizeof(VecHeader);
        VECHEAD(*vec)->cap *= 2;
    }

    char* dest = (char*)*vec;
    memcpy(&dest[VECHEAD(*vec)->len], el, VECHEAD(*vec)->sizeof_type);
    VECHEAD(*vec)->len++;

    return *vec;
}

inline size_t vecLen(void* vec) {
    return VECHEAD(vec)->len;
}

inline void vecReset(void* vec) {
    VECHEAD(vec)->cur = NULL;
}

inline void vecEmpty(void* vec) {
    VECHEAD(vec)->len = 0;
}