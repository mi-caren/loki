#ifndef VEC_H
#define VEC_H

#include <stdbool.h>
#include <stdlib.h>

#include "utils.h"

#define VecType(TYPE)\
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

#define VEC_STRUCT_DEF(TYPE)\
    typedef struct {\
        size_t cap;\
        size_t len;\
        size_t curr;\
        TYPE* items;\
    } VecType(TYPE)

#define Vec(TYPE)                       VecType(TYPE)*

/* ********* vec_new *********** */
#define VEC_NEW_FUNC_NAME(TYPE)         CAT(VecType(TYPE), _new)
#define VEC_NEW_FUNC_SIGNATURE(TYPE)    Vec(TYPE) VEC_NEW_FUNC_NAME(TYPE)(size_t initial_size)
#define VEC_NEW_FUNC_IMPL(TYPE)\
    VEC_NEW_FUNC_SIGNATURE(TYPE) {\
        Vec(TYPE) vec = malloc(sizeof(VecType(TYPE)));\
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
        return vec;\
    }

#define vec_new(TYPE)                   VEC_NEW_FUNC_NAME(TYPE)(1)
#define vec_new_with_cap(TYPE, CAP)     VEC_NEW_FUNC_NAME(TYPE)(CAP)

/* ********* vec_begin *********** */
#define VEC_BEGIN_FUNC_NAME(TYPE)           CAT(VecType(TYPE), _begin)
#define VEC_BEGIN_FUNC_SIGNATURE(TYPE)      TYPE* VEC_BEGIN_FUNC_NAME(TYPE)(Vec(TYPE) vec)
#define VEC_BEGIN_FUNC_IMPL(TYPE)\
    VEC_BEGIN_FUNC_SIGNATURE(TYPE) {\
        vec->curr = 0;\
        return vec_curr(TYPE, vec);\
    }

#define vec_begin(TYPE, VEC)                VEC_BEGIN_FUNC_NAME(TYPE)(VEC)

/* ********* vec_end *********** */
#define VEC_END_FUNC_NAME(TYPE)           CAT(VecType(TYPE), _end)
#define VEC_END_FUNC_SIGNATURE(TYPE)      TYPE* VEC_END_FUNC_NAME(TYPE)(Vec(TYPE) vec)
#define VEC_END_FUNC_IMPL(TYPE)\
    VEC_END_FUNC_SIGNATURE(TYPE) {\
        if (vec->len == 0) return NULL;\
        vec->curr = vec->len - 1;\
        return vec_curr(TYPE, vec);\
    }

#define vec_end(TYPE, VEC)                VEC_END_FUNC_NAME(TYPE)(VEC)

/* ********* vec_curr *********** */
#define VEC_CURR_FUNC_NAME(TYPE)            CAT(VecType(TYPE), _curr)
#define VEC_CURR_FUNC_SIGNATURE(TYPE)       TYPE* VEC_CURR_FUNC_NAME(TYPE)(Vec(TYPE) vec)
#define VEC_CURR_FUNC_IMPL(TYPE)\
    VEC_CURR_FUNC_SIGNATURE(TYPE) {\
        if (vec->len == 0) return NULL;\
        if (vec->curr >= vec->len) return NULL;\
        return &vec->items[vec->curr];\
    }

#define vec_curr(TYPE, VEC)                 VEC_CURR_FUNC_NAME(TYPE)(VEC)

/* ********* vec_next *********** */
#define VEC_NEXT_FUNC_NAME(TYPE)            CAT(VecType(TYPE), _next)
#define VEC_NEXT_FUNC_SIGNATURE(TYPE)       TYPE* VEC_NEXT_FUNC_NAME(TYPE)(Vec(TYPE) vec)
#define VEC_NEXT_FUNC_IMPL(TYPE)\
    VEC_NEXT_FUNC_SIGNATURE(TYPE) {\
        if (vec->curr >= vec->len - 1) return NULL;\
        vec->curr++;\
        return vec_curr(TYPE, vec);\
    }

#define vec_next(TYPE, VEC)                 VEC_NEXT_FUNC_NAME(TYPE)(VEC)

/* ********* vec_prev *********** */
#define VEC_PREV_FUNC_NAME(TYPE)            CAT(VecType(TYPE), _prev)
#define VEC_PREV_FUNC_SIGNATURE(TYPE)       TYPE* VEC_PREV_FUNC_NAME(TYPE)(Vec(TYPE) vec)
#define VEC_PREV_FUNC_IMPL(TYPE)\
    VEC_PREV_FUNC_SIGNATURE(TYPE) {\
        if (vec->curr == 0) return NULL;\
        vec->curr--;\
        return vec_curr(TYPE, vec);\
    }

#define vec_prev(TYPE, VEC)                 VEC_PREV_FUNC_NAME(TYPE)(VEC)

/* ********* vec_empty *********** */
#define VEC_EMPTY_FUNC_NAME(TYPE)            CAT(VecType(TYPE), _empty)
#define VEC_EMPTY_FUNC_SIGNATURE(TYPE)       void VEC_EMPTY_FUNC_NAME(TYPE)(Vec(TYPE) vec)
#define VEC_EMPTY_FUNC_IMPL(TYPE)\
    inline VEC_EMPTY_FUNC_SIGNATURE(TYPE) {\
        vec->len = 0;\
    }

#define vec_empty(TYPE, VEC)                 VEC_EMPTY_FUNC_NAME(TYPE)(VEC)

/* ********* vec_push *********** */
#define VEC_PUSH_FUNC_NAME(TYPE)            CAT(VecType(TYPE), _push)
#define VEC_PUSH_FUNC_SIGNATURE(TYPE)       Vec(TYPE) VEC_PUSH_FUNC_NAME(TYPE)(Vec(TYPE) vec, TYPE el)
#define VEC_PUSH_FUNC_IMPL(TYPE)\
    VEC_PUSH_FUNC_SIGNATURE(TYPE) {\
        if (vec->len == vec->cap) {\
            if (vec_grow(TYPE, vec) == NULL)\
                return NULL;\
        }\
        vec->items[vec->len] = el;\
        vec->len++;\
        return vec;\
    }

#define vec_push(TYPE, VEC, EL)             VEC_PUSH_FUNC_NAME(TYPE)(VEC, EL)

/* ********* static vec_grow *********** */
#define vec_grow(TYPE, VEC)                 vec_realloc(TYPE, VEC, VEC->cap * 2)

/* ********* vec_repeat_append *********** */
#define VEC_REPEAT_APPEND_FUNC_NAME(TYPE)            CAT(VecType(TYPE), _repeat_append)
#define VEC_REPEAT_APPEND_FUNC_SIGNATURE(TYPE)       Vec(TYPE) VEC_REPEAT_APPEND_FUNC_NAME(TYPE)(Vec(TYPE) vec, TYPE el, size_t n)
#define VEC_REPEAT_APPEND_FUNC_IMPL(TYPE)\
    VEC_REPEAT_APPEND_FUNC_SIGNATURE(TYPE) {\
        size_t total_space = vec->len + n;\
        if (total_space > vec->cap) {\
            if (vec_make_space(TYPE, vec, total_space) == NULL)\
                return NULL;\
        }\
\
        for (size_t i = 0; i < n; i++) {\
            vec->items[vec->len + i] = el;\
        }\
        vec->len += n;\
\
        return vec;\
    }

#define vec_repeat_append(TYPE, VEC, EL, N)                 VEC_REPEAT_APPEND_FUNC_NAME(TYPE)(VEC, EL, N)

/* ********* static vec_make_space *********** */
#define vec_make_space(TYPE, VEC, SPACE)                 vec_realloc(TYPE, VEC, vec_cap_from_size(SPACE))

/* ********* static vec_realloc *********** */
#define VEC_REALLOC_FUNC_NAME(TYPE)            CAT(VecType(TYPE), _realloc)
#define VEC_REALLOC_FUNC_SIGNATURE(TYPE)       Vec(TYPE) VEC_REALLOC_FUNC_NAME(TYPE)(Vec(TYPE) vec, size_t size)
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

#define vec_realloc(TYPE, VEC, SIZE)                 VEC_REALLOC_FUNC_NAME(TYPE)(VEC, SIZE)

/* ********* vec_set *********** */
#define VEC_SET_FUNC_NAME(TYPE)            CAT(VecType(TYPE), _set)
#define VEC_SET_FUNC_SIGNATURE(TYPE)       TYPE* VEC_SET_FUNC_NAME(TYPE)(Vec(TYPE) vec, TYPE val, size_t idx)
#define VEC_SET_FUNC_IMPL(TYPE)\
    VEC_SET_FUNC_SIGNATURE(TYPE) {\
        if (idx >= vec->len) return NULL;\
        vec->items[idx] = val;\
        return &vec->items[idx];\
    }

#define vec_set(TYPE, VEC, VAL, IDX)        VEC_SET_FUNC_NAME(TYPE)(VEC, VAL, IDX)

/* ********* vec_get *********** */
#define VEC_GET_FUNC_NAME(TYPE)            CAT(VecType(TYPE), _get)
#define VEC_GET_FUNC_SIGNATURE(TYPE)       TYPE* VEC_GET_FUNC_NAME(TYPE)(Vec(TYPE) vec, size_t idx)
#define VEC_GET_FUNC_IMPL(TYPE)\
    VEC_GET_FUNC_SIGNATURE(TYPE) {\
        if (idx >= vec->len) return NULL;\
        return &vec->items[idx];\
    }

#define vec_get(TYPE, VEC, IDX)        VEC_GET_FUNC_NAME(TYPE)(VEC, IDX)

/* ********* vec_items *********** */
#define VEC_ITEMS_FUNC_NAME(TYPE)            CAT(VecType(TYPE), _items)
#define VEC_ITEMS_FUNC_SIGNATURE(TYPE)       TYPE* VEC_ITEMS_FUNC_NAME(TYPE)(Vec(TYPE) vec)
#define VEC_ITEMS_FUNC_IMPL(TYPE)\
    inline VEC_ITEMS_FUNC_SIGNATURE(TYPE) {\
        return vec->items;\
    }

#define vec_items(TYPE, VEC)        VEC_ITEMS_FUNC_NAME(TYPE)(VEC)

/* ********* vec_insert *********** */
#define VEC_INSERT_FUNC_NAME(TYPE)            CAT(VecType(TYPE), _insert)
#define VEC_INSERT_FUNC_SIGNATURE(TYPE)       Vec(TYPE) VEC_INSERT_FUNC_NAME(TYPE)(Vec(TYPE) vec, TYPE el, size_t pos)
#define VEC_INSERT_FUNC_IMPL(TYPE)\
    VEC_INSERT_FUNC_SIGNATURE(TYPE) {\
        if (pos > vec->len)\
            return NULL;\
        if (vec->len == vec->cap) {\
            if (vec_grow(TYPE, vec) == NULL)\
                return NULL;\
        }\
        memmove(\
            &vec->items[pos+1],\
            &vec->items[pos],\
            sizeof(TYPE) * (vec->len - pos)\
        );\
        vec->items[pos] = el;\
        vec->len++;\
        return vec;\
    }

#define vec_insert(TYPE, VEC, EL, POS)             VEC_INSERT_FUNC_NAME(TYPE)(VEC, EL, POS)

/* ********* vec_remove *********** */
#define VEC_REMOVE_FUNC_NAME(TYPE)            CAT(VecType(TYPE), _remove)
#define VEC_REMOVE_FUNC_SIGNATURE(TYPE)       Vec(TYPE) VEC_REMOVE_FUNC_NAME(TYPE)(Vec(TYPE) vec, size_t pos)
#define VEC_REMOVE_FUNC_IMPL(TYPE)\
    VEC_REMOVE_FUNC_SIGNATURE(TYPE) {\
        if (pos >= vec->len) return NULL;\
        memmove(\
            &vec->items[pos],\
            &vec->items[pos+1],\
            sizeof(TYPE) * (vec->len - pos - 1)\
        );\
        vec->len--;\
        return vec;\
    }

#define vec_remove(TYPE, VEC, POS)        VEC_REMOVE_FUNC_NAME(TYPE)(VEC, POS)

/* ********* vec_last *********** */
#define VEC_LAST_FUNC_NAME(TYPE)           CAT(VecType(TYPE), _last)
#define VEC_LAST_FUNC_SIGNATURE(TYPE)      TYPE* VEC_LAST_FUNC_NAME(TYPE)(Vec(TYPE) vec)
#define VEC_LAST_FUNC_IMPL(TYPE)\
    VEC_LAST_FUNC_SIGNATURE(TYPE) {\
        if (vec->len == 0) return NULL;\
        return &vec->items[vec->len - 1];\
    }

#define vec_last(TYPE, VEC)                VEC_LAST_FUNC_NAME(TYPE)(VEC)

/* ********* vec_first *********** */
#define VEC_FIRST_FUNC_NAME(TYPE)           CAT(VecType(TYPE), _first)
#define VEC_FIRST_FUNC_SIGNATURE(TYPE)      TYPE* VEC_FIRST_FUNC_NAME(TYPE)(Vec(TYPE) vec)
#define VEC_FIRST_FUNC_IMPL(TYPE)\
    VEC_FIRST_FUNC_SIGNATURE(TYPE) {\
        if (vec->len == 0) return NULL;\
        return &vec->items[0];\
    }

#define vec_first(TYPE, VEC)                VEC_FIRST_FUNC_NAME(TYPE)(VEC)

/* ********* vec_pop *********** */
#define VEC_POP_FUNC_NAME(TYPE)           CAT(VecType(TYPE), _pop)
#define VEC_POP_FUNC_SIGNATURE(TYPE)      TYPE* VEC_POP_FUNC_NAME(TYPE)(Vec(TYPE) vec)
#define VEC_POP_FUNC_IMPL(TYPE)\
    VEC_POP_FUNC_SIGNATURE(TYPE) {\
        if (vec->len == 0) return NULL;\
        TYPE* el = vec_last(TYPE, vec);\
        vec->len--;\
        if (vec->curr >= vec->len)\
            vec->curr = vec->len - 1;\
        return el;\
    }

#define vec_pop(TYPE, VEC)                VEC_POP_FUNC_NAME(TYPE)(VEC)

/* ********* vec_free *********** */
#define VEC_FREE_FUNC_NAME(TYPE)           CAT(VecType(TYPE), _free)
#define VEC_FREE_FUNC_SIGNATURE(TYPE)      void VEC_FREE_FUNC_NAME(TYPE)(Vec(TYPE) vec)
#define VEC_FREE_FUNC_IMPL(TYPE)\
    VEC_FREE_FUNC_SIGNATURE(TYPE) {\
        free(vec->items);\
        free(vec);\
    }

#define vec_free(TYPE, VEC)                VEC_FREE_FUNC_NAME(TYPE)(VEC)


#define VEC_DEFS(TYPE)\
    VEC_STRUCT_DEF(TYPE);\
    VEC_NEW_FUNC_SIGNATURE(TYPE);\
    VEC_BEGIN_FUNC_SIGNATURE(TYPE);\
    VEC_END_FUNC_SIGNATURE(TYPE);\
    VEC_CURR_FUNC_SIGNATURE(TYPE);\
    VEC_NEXT_FUNC_SIGNATURE(TYPE);\
    VEC_PREV_FUNC_SIGNATURE(TYPE);\
    VEC_EMPTY_FUNC_SIGNATURE(TYPE);\
    VEC_PUSH_FUNC_SIGNATURE(TYPE);\
    VEC_REPEAT_APPEND_FUNC_SIGNATURE(TYPE);\
    VEC_SET_FUNC_SIGNATURE(TYPE);\
    VEC_GET_FUNC_SIGNATURE(TYPE);\
    VEC_ITEMS_FUNC_SIGNATURE(TYPE);\
    VEC_INSERT_FUNC_SIGNATURE(TYPE);\
    VEC_REMOVE_FUNC_SIGNATURE(TYPE);\
    VEC_LAST_FUNC_SIGNATURE(TYPE);\
    VEC_FIRST_FUNC_SIGNATURE(TYPE);\
    VEC_POP_FUNC_SIGNATURE(TYPE);\
    VEC_FREE_FUNC_SIGNATURE(TYPE);\

#define VEC_IMPL(TYPE)\
    static VEC_REALLOC_FUNC_SIGNATURE(TYPE);\
    VEC_NEW_FUNC_IMPL(TYPE)\
    VEC_BEGIN_FUNC_IMPL(TYPE)\
    VEC_END_FUNC_IMPL(TYPE)\
    VEC_CURR_FUNC_IMPL(TYPE)\
    VEC_NEXT_FUNC_IMPL(TYPE)\
    VEC_PREV_FUNC_IMPL(TYPE)\
    VEC_EMPTY_FUNC_IMPL(TYPE)\
    VEC_PUSH_FUNC_IMPL(TYPE)\
    VEC_REPEAT_APPEND_FUNC_IMPL(TYPE)\
    VEC_SET_FUNC_IMPL(TYPE)\
    VEC_GET_FUNC_IMPL(TYPE)\
    VEC_ITEMS_FUNC_IMPL(TYPE)\
    VEC_INSERT_FUNC_IMPL(TYPE)\
    VEC_REMOVE_FUNC_IMPL(TYPE)\
    VEC_LAST_FUNC_IMPL(TYPE)\
    VEC_FIRST_FUNC_IMPL(TYPE)\
    VEC_POP_FUNC_IMPL(TYPE)\
    VEC_FREE_FUNC_IMPL(TYPE)\
    static VEC_REALLOC_FUNC_IMPL(TYPE)\

#define VEC_FOREACH(TYPE, EL, VEC) \
    for (TYPE* EL = vec_begin(TYPE, VEC); EL != NULL; EL = vec_next(TYPE, VEC))

#define VEC_FOREACH_REV(TYPE, EL, VEC) \
    for (TYPE* EL = vec_end(TYPE, VEC); EL != NULL; EL = vec_prev(TYPE, VEC))

size_t vec_cap_from_size(size_t size);

VEC_DEFS(char)


#endif