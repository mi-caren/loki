#ifndef ITERATOR_H
#define ITERATOR_H

#define InterfaceName(NAME, TYPE)   CAT(CAT(TYPE, NAME), Interface)

#define IteratorType(SELF_TYPE) CAT(SELF_TYPE, Iterator)

#define ITERATOR_INTERFACE_DEF(SELF_TYPE, ITEM_TYPE)\
    typedef struct {\
        ITEM_TYPE* (*const begin) (SELF_TYPE* const self);\
        ITEM_TYPE* (*const end)   (SELF_TYPE* const self);\
        ITEM_TYPE* (*const curr)  (SELF_TYPE* const self);\
        ITEM_TYPE* (*const next)  (SELF_TYPE* const self);\
        ITEM_TYPE* (*const prev)  (SELF_TYPE* const self);\
    } InterfaceName(Iterator, SELF_TYPE)

#define IterableStructName(SELF_TYPE)    CAT(Iterable, SELF_TYPE)

#define ITERABLE_DEF(SELF_TYPE, ITEM_TYPE)\
    typedef struct {\
        SELF_TYPE* const self;\
        const InterfaceName(Iterator, SELF_TYPE)* const iterator;\
    } IterableStructName(SELF_TYPE)



#define ITER_FUNC_NAME(SELF_TYPE)       CAT(SELF_TYPE, _iter)
#define ITER_FUNC_SIGNATURE(SELF_TYPE)  IterableStructName(SELF_TYPE) ITER_FUNC_NAME(SELF_TYPE)(SELF_TYPE* self)
#define ITER_FUNC_IMPL(SELF_TYPE, BEGIN_IMPL, END_IMPL, CURR_IMPL, NEXT_IMPL, PREV_IMPL)\
    ITER_FUNC_SIGNATURE(SELF_TYPE) {\
        static const InterfaceName(Iterator, SELF_TYPE) iterator = {\
            .begin  = BEGIN_IMPL,\
            .end    = END_IMPL,\
            .curr   = CURR_IMPL,\
            .next   = NEXT_IMPL,\
            .prev   = PREV_IMPL,\
        };\
        return (IterableStructName(SELF_TYPE)) {\
            .self       = self,\
            .iterator   = &iterator,\
        };\
    }

#define iter(SELF_TYPE, SELF)   ITER_FUNC_NAME(SELF_TYPE)(SELF)



#define ITER_DEFS(SELF_TYPE, ITEM_TYPE)\
    ITERATOR_INTERFACE_DEF(SELF_TYPE, ITEM_TYPE);\
    ITERABLE_DEF(SELF_TYPE, ITEM_TYPE);\
    ITER_FUNC_SIGNATURE(SELF_TYPE);\

#define ITER_IMPL(SELF_TYPE, BEGIN_IMPL, END_IMPL, CURR_IMPL, NEXT_IMPL, PREV_IMPL)\
    ITER_FUNC_IMPL(SELF_TYPE, BEGIN_IMPL, END_IMPL, CURR_IMPL, NEXT_IMPL, PREV_IMPL)\


#endif