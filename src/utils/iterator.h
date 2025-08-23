#ifndef ITERATOR_H
#define ITERATOR_H

// SELF_TYPE* const self;

#define IteratorType(SELF_TYPE) CAT(SELF_TYPE, Iterator)

#define ITER_STRUCT_DEF(SELF_TYPE, ITEM_TYPE)\
    typedef struct {\
        ITEM_TYPE* (*const begin) (SELF_TYPE* const self);\
        ITEM_TYPE* (*const end)   (SELF_TYPE* const self);\
        ITEM_TYPE* (*const curr)  (SELF_TYPE* const self);\
        ITEM_TYPE* (*const next)  (SELF_TYPE* const self);\
        ITEM_TYPE* (*const prev)  (SELF_TYPE* const self);\
    } IteratorType(SELF_TYPE)

#endif