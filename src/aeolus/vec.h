#ifndef VEC_H
#define VEC_H

#include <stdbool.h>
#include <stdlib.h>

#include "utils.h"
#include "generics.h"
#include "iterator.h"

#define Vec(TYPE) GenericName(TYPE, Vec)

#define VEC_STRUCT_DEF(TYPE)\
    typedef struct {\
        size_t cap;\
        size_t len;\
        size_t curr;\
        TYPE* items;\
        const struct VecDriver(TYPE)* drv;\
        ITER_DRIVER(Vec(TYPE));\
    } Vec(TYPE)

#define VecDriver(TYPE)     CAT(Vec(TYPE), Driver)


#define VEC_DRIVER_DEF(TYPE)\
    struct VecDriver(TYPE) {\
        void       (*empty)         (Vec(TYPE)* self);\
        Vec(TYPE)* (*push)          (Vec(TYPE)* self, TYPE el);\
        TYPE*      (*pop)           (Vec(TYPE)* self);\
        Vec(TYPE)* (*repeat_append) (Vec(TYPE)* self, TYPE el, size_t n);\
        TYPE*      (*set)           (Vec(TYPE)* self, TYPE val, size_t idx);\
        TYPE*      (*get)           (Vec(TYPE)* self, size_t idx);\
        Vec(TYPE)* (*insert)        (Vec(TYPE)* self, TYPE el, size_t pos);\
        Vec(TYPE)* (*remove)        (Vec(TYPE)* self, size_t pos);\
        TYPE*      (*last)          (Vec(TYPE)* self);\
        TYPE*      (*first)         (Vec(TYPE)* self);\
        void       (*free)          (Vec(TYPE)* self);\
    }

/* ********* vec_new *********** */
#define VEC_NEW_FUNC_NAME(TYPE)         CAT(Vec(TYPE), _new)
#define VEC_NEW_FUNC_SIGNATURE(TYPE)    Vec(TYPE)* VEC_NEW_FUNC_NAME(TYPE)(size_t initial_size)
#define VEC_NEW_FUNC_IMPL(TYPE)\
    VEC_NEW_FUNC_SIGNATURE(TYPE) {\
        Vec(TYPE)* vec = malloc(sizeof(Vec(TYPE)));\
        if (vec == NULL) return NULL;\
        size_t cap = vec_cap_from_size(initial_size);\
        TYPE* items = malloc(sizeof(TYPE) * cap);\
        if (items == NULL) {\
            free(vec);\
            return NULL;\
        }\
        vec->cap = cap;\
        vec->len = 0;\
        vec->curr = 0;\
        vec->items = items;\
        static const struct VecDriver(TYPE) vec_driver = {\
            .empty         = VEC_EMPTY_FUNC_NAME(TYPE),\
            .push          = VEC_PUSH_FUNC_NAME(TYPE),\
            .pop           = VEC_POP_FUNC_NAME(TYPE),\
            .repeat_append = VEC_REPEAT_APPEND_FUNC_NAME(TYPE),\
            .set           = VEC_SET_FUNC_NAME(TYPE),\
            .get           = VEC_GET_FUNC_NAME(TYPE),\
            .insert        = VEC_INSERT_FUNC_NAME(TYPE),\
            .remove        = VEC_REMOVE_FUNC_NAME(TYPE),\
            .last          = VEC_LAST_FUNC_NAME(TYPE),\
            .first         = VEC_FIRST_FUNC_NAME(TYPE),\
            .free          = VEC_FREE_FUNC_NAME(TYPE),\
        };\
        vec->drv = &vec_driver;\
        ITER_INIT(Vec(TYPE), vec);\
        return vec;\
    }

#define vec_new(TYPE)                   VEC_NEW_FUNC_NAME(TYPE)(1)
#define vec_new_with_cap(TYPE, CAP)     VEC_NEW_FUNC_NAME(TYPE)(CAP)

/* ********* vec_empty *********** */
#define VEC_EMPTY_FUNC_NAME(TYPE)            CAT(Vec(TYPE), _empty)
#define VEC_EMPTY_FUNC_SIGNATURE(TYPE)       void VEC_EMPTY_FUNC_NAME(TYPE)(Vec(TYPE)* self)
#define VEC_EMPTY_FUNC_IMPL(TYPE)\
    VEC_EMPTY_FUNC_SIGNATURE(TYPE) {\
        self->len = 0;\
    }

#define vec_empty(SELF)     (SELF)->drv->empty(SELF)

/* ********* vec_push *********** */
#define VEC_PUSH_FUNC_NAME(TYPE)            CAT(Vec(TYPE), _push)
#define VEC_PUSH_FUNC_SIGNATURE(TYPE)       Vec(TYPE)* VEC_PUSH_FUNC_NAME(TYPE)(Vec(TYPE)* self, TYPE el)
#define VEC_PUSH_FUNC_IMPL(TYPE)\
    VEC_PUSH_FUNC_SIGNATURE(TYPE) {\
        if (self->len == self->cap) {\
            if (vec_grow_fn(TYPE, self) == NULL)\
                return NULL;\
        }\
        self->items[self->len] = el;\
        self->len++;\
        return self;\
    }

#define vec_push(SELF, EL)             (SELF)->drv->push(SELF, EL)

/* ********* static vec_grow *********** */
#define vec_grow_fn(TYPE, VEC)                 vec_realloc_fn(TYPE, VEC, VEC->cap * 2)

/* ********* vec_repeat_append *********** */
#define VEC_REPEAT_APPEND_FUNC_NAME(TYPE)            CAT(Vec(TYPE), _repeat_append)
#define VEC_REPEAT_APPEND_FUNC_SIGNATURE(TYPE)       Vec(TYPE)* VEC_REPEAT_APPEND_FUNC_NAME(TYPE)(Vec(TYPE)* self, TYPE el, size_t n)
#define VEC_REPEAT_APPEND_FUNC_IMPL(TYPE)\
    VEC_REPEAT_APPEND_FUNC_SIGNATURE(TYPE) {\
        size_t total_space = self->len + n;\
        if (total_space > self->cap) {\
            if (vec_make_space_fn(TYPE, self, total_space) == NULL)\
                return NULL;\
        }\
\
        for (size_t i = 0; i < n; i++) {\
            self->items[self->len + i] = el;\
        }\
        self->len += n;\
        return self;\
    }

#define vec_repeat_append(SELF, EL, N)       (SELF)->drv->repeat_append(SELF, EL, N)

/* ********* static vec_make_space *********** */
#define vec_make_space_fn(TYPE, VEC, SPACE)                 vec_realloc_fn(TYPE, VEC, vec_cap_from_size(SPACE))

/* ********* static vec_realloc *********** */
#define VEC_REALLOC_FUNC_NAME(TYPE)            CAT(Vec(TYPE), _realloc)
#define VEC_REALLOC_FUNC_SIGNATURE(TYPE)       Vec(TYPE)* VEC_REALLOC_FUNC_NAME(TYPE)(Vec(TYPE)* vec, size_t size)
#define VEC_REALLOC_FUNC_IMPL(TYPE)\
    VEC_REALLOC_FUNC_SIGNATURE(TYPE) {\
        if (vec == NULL || vec->items == NULL) return NULL;\
        TYPE* new = realloc(\
            vec->items,\
            sizeof(TYPE) * size\
        );\
        if (new == NULL) return NULL;\
        vec->items = new;\
        vec->cap = size;\
        return vec;\
    }

#define vec_realloc_fn(TYPE, VEC, SIZE)                 VEC_REALLOC_FUNC_NAME(TYPE)(VEC, SIZE)

/* ********* vec_set *********** */
#define VEC_SET_FUNC_NAME(TYPE)            CAT(Vec(TYPE), _set)
#define VEC_SET_FUNC_SIGNATURE(TYPE)       TYPE* VEC_SET_FUNC_NAME(TYPE)(Vec(TYPE)* self, TYPE val, size_t pos)
#define VEC_SET_FUNC_IMPL(TYPE)\
    VEC_SET_FUNC_SIGNATURE(TYPE) {\
        if (pos >= self->len) return NULL;\
        self->items[pos] = val;\
        return &self->items[pos];\
    }

#define vec_set(SELF, VAL, POS)        (SELF)->drv->set(SELF, VAL, POS)

/* ********* vec_get *********** */
#define VEC_GET_FUNC_NAME(TYPE)            CAT(Vec(TYPE), _get)
#define VEC_GET_FUNC_SIGNATURE(TYPE)       TYPE* VEC_GET_FUNC_NAME(TYPE)(Vec(TYPE)* self, size_t pos)
#define VEC_GET_FUNC_IMPL(TYPE)\
    VEC_GET_FUNC_SIGNATURE(TYPE) {\
        if (pos >= self->len) return NULL;\
        return &self->items[pos];\
    }

#define vec_get(SELF, POS)        (SELF)->drv->get(SELF, POS)

/* ********* vec_insert *********** */
#define VEC_INSERT_FUNC_NAME(TYPE)            CAT(Vec(TYPE), _insert)
#define VEC_INSERT_FUNC_SIGNATURE(TYPE)       Vec(TYPE)* VEC_INSERT_FUNC_NAME(TYPE)(Vec(TYPE)* self, TYPE el, size_t pos)
#define VEC_INSERT_FUNC_IMPL(TYPE)\
    VEC_INSERT_FUNC_SIGNATURE(TYPE) {\
        if (pos > self->len)\
            return NULL;\
        if (self->len == self->cap) {\
            if (vec_grow_fn(TYPE, self) == NULL)\
                return NULL;\
        }\
        memmove(\
            &self->items[pos+1],\
            &self->items[pos],\
            sizeof(TYPE) * (self->len - pos)\
        );\
        self->items[pos] = el;\
        self->len++;\
        return self;\
    }

#define vec_insert(SELF, EL, POS)             (SELF)->drv->insert(SELF, EL, POS)

/* ********* vec_remove *********** */
#define VEC_REMOVE_FUNC_NAME(TYPE)            CAT(Vec(TYPE), _remove)
#define VEC_REMOVE_FUNC_SIGNATURE(TYPE)       Vec(TYPE)* VEC_REMOVE_FUNC_NAME(TYPE)(Vec(TYPE)* self, size_t pos)
#define VEC_REMOVE_FUNC_IMPL(TYPE)\
    VEC_REMOVE_FUNC_SIGNATURE(TYPE) {\
        if (pos >= self->len) return NULL;\
        memmove(\
            &self->items[pos],\
            &self->items[pos+1],\
            sizeof(TYPE) * (self->len - pos - 1)\
        );\
        self->len--;\
        return self;\
    }

#define vec_remove(SELF, POS)        (SELF)->drv->remove(SELF, POS)

/* ********* vec_last *********** */
#define VEC_LAST_FUNC_NAME(TYPE)           CAT(Vec(TYPE), _last)
#define VEC_LAST_FUNC_SIGNATURE(TYPE)      TYPE* VEC_LAST_FUNC_NAME(TYPE)(Vec(TYPE)* self)
#define VEC_LAST_FUNC_IMPL(TYPE)\
    VEC_LAST_FUNC_SIGNATURE(TYPE) {\
        if (self->len == 0) return NULL;\
        return &self->items[self->len - 1];\
    }

#define vec_last(SELF)                (SELF)->drv->last(SELF)
#define vec_last_fn(TYPE, VEC)        VEC_LAST_FUNC_NAME(TYPE)(VEC)

/* ********* vec_first *********** */
#define VEC_FIRST_FUNC_NAME(TYPE)           CAT(Vec(TYPE), _first)
#define VEC_FIRST_FUNC_SIGNATURE(TYPE)      TYPE* VEC_FIRST_FUNC_NAME(TYPE)(Vec(TYPE)* self)
#define VEC_FIRST_FUNC_IMPL(TYPE)\
    VEC_FIRST_FUNC_SIGNATURE(TYPE) {\
        if (self->len == 0) return NULL;\
        return &self->items[0];\
    }

#define vec_first(SELF)                (SELF)->drv->first(SELF)

/* ********* vec_pop *********** */
#define VEC_POP_FUNC_NAME(TYPE)           CAT(Vec(TYPE), _pop)
#define VEC_POP_FUNC_SIGNATURE(TYPE)      TYPE* VEC_POP_FUNC_NAME(TYPE)(Vec(TYPE)* self)
#define VEC_POP_FUNC_IMPL(TYPE)\
    VEC_POP_FUNC_SIGNATURE(TYPE) {\
        if (self->len == 0) return NULL;\
        TYPE* el = vec_last_fn(TYPE, self);\
        self->len--;\
        if (self->curr >= self->len)\
            self->curr = self->len - 1;\
        return el;\
    }

#define vec_pop(SELF)                (SELF)->drv->pop(SELF)

/* ********* vec_free *********** */
#define VEC_FREE_FUNC_NAME(TYPE)           CAT(Vec(TYPE), _free)
#define VEC_FREE_FUNC_SIGNATURE(TYPE)      void VEC_FREE_FUNC_NAME(TYPE)(Vec(TYPE)* self)
#define VEC_FREE_FUNC_IMPL(TYPE)\
    VEC_FREE_FUNC_SIGNATURE(TYPE) {\
        free(self->items);\
        free(self);\
    }

#define vec_free(SELF)                (SELF)->drv->free(SELF)


#define VEC_DEFS(TYPE)\
    struct VecDriver(TYPE);\
    struct Iterator(Vec(TYPE));\
    VEC_STRUCT_DEF(TYPE);\
    VEC_DRIVER_DEF(TYPE);\
    VEC_NEW_FUNC_SIGNATURE(TYPE);\
    ITER_DEFS(Vec(TYPE), TYPE)\

#define VEC_IMPL(TYPE)\
    static VEC_REALLOC_FUNC_SIGNATURE(TYPE);\
    VEC_EMPTY_FUNC_IMPL(TYPE)\
    VEC_PUSH_FUNC_IMPL(TYPE)\
    VEC_REPEAT_APPEND_FUNC_IMPL(TYPE)\
    VEC_SET_FUNC_IMPL(TYPE)\
    VEC_GET_FUNC_IMPL(TYPE)\
    VEC_INSERT_FUNC_IMPL(TYPE)\
    VEC_REMOVE_FUNC_IMPL(TYPE)\
    VEC_LAST_FUNC_IMPL(TYPE)\
    VEC_FIRST_FUNC_IMPL(TYPE)\
    VEC_POP_FUNC_IMPL(TYPE)\
    VEC_FREE_FUNC_IMPL(TYPE)\
    ITER_IMPL(\
        Vec(TYPE),\
        /* CURR_IMPL */ {\
            if (self->len == 0) return NULL;\
            if (self->curr >= self->len) return NULL;\
            return &self->items[self->curr];\
        },\
        /* BEGIN_IMPL */ {\
            self->curr = 0;\
            return iter_curr(self);\
        },\
        /* END_IMPL */ {\
            if (self->len == 0) return NULL;\
            self->curr = self->len - 1;\
            return iter_curr(self);\
        },\
        /* PREV_IMPL */ {\
            if (self->curr == 0) return NULL;\
            self->curr--;\
            return iter_curr(self);\
        },\
        /* NEXT_IMPL */ {\
            if (self->curr >= self->len - 1) return NULL;\
            self->curr++;\
            return iter_curr(self);\
        }\
    )\
    VEC_NEW_FUNC_IMPL(TYPE)\
    static VEC_REALLOC_FUNC_IMPL(TYPE)\

size_t vec_cap_from_size(size_t size);

VEC_DEFS(char)
VEC_DEFS(unsigned int)


#endif