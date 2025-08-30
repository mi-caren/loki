#ifndef ITERATOR_H
#define ITERATOR_H

#define Iterator(ITERABLE)          CAT(ITERABLE, Iterator)
#define IterItem(ITERABLE)          CAT(Iterator(ITERABLE), Item)

#define ITER_DRIVER_DEF(ITERABLE)\
    struct Iterator(ITERABLE) {\
        IterItem(ITERABLE)* (*curr)  (ITERABLE* const self);\
        IterItem(ITERABLE)* (*begin) (ITERABLE* const self);\
        IterItem(ITERABLE)* (*end)   (ITERABLE* const self);\
        IterItem(ITERABLE)* (*prev)  (ITERABLE* const self);\
        IterItem(ITERABLE)* (*next)  (ITERABLE* const self);\
    }

#define ITER_DRIVER(ITERABLE)   const struct Iterator(ITERABLE)* iter
#define ITER_INIT(ITERABLE, IT)\
    static const struct Iterator(ITERABLE)\
    CAT(Iterator(ITERABLE), _iter) = {\
        .curr  = ITER_CURR_FUNC_NAME(ITERABLE),\
        .begin = ITER_BEGIN_FUNC_NAME(ITERABLE),\
        .end   = ITER_END_FUNC_NAME(ITERABLE),\
        .prev  = ITER_PREV_FUNC_NAME(ITERABLE),\
        .next  = ITER_NEXT_FUNC_NAME(ITERABLE),\
    };\
    (IT)->iter = &CAT(Iterator(ITERABLE), _iter)

#define ITER_GET_DRIVER()

/* ********* iter_curr *********** */
#define ITER_CURR_FUNC_NAME(ITERABLE)      CAT(Iterator(ITERABLE), _curr)
#define ITER_CURR_FUNC_SIGNATURE(ITERABLE) IterItem(ITERABLE)* ITER_CURR_FUNC_NAME(ITERABLE)(ITERABLE* const self)
#define ITER_CURR_FUNC_IMPL(ITERABLE)\
    ITER_CURR_FUNC_SIGNATURE(ITERABLE)

#define iter_curr(SELF)                 (SELF)->iter->curr(SELF)

/* ********* iter_begin *********** */
#define ITER_BEGIN_FUNC_NAME(ITERABLE)       CAT(Iterator(ITERABLE), _begin)
#define ITER_BEGIN_FUNC_SIGNATURE(ITERABLE)  IterItem(ITERABLE)* ITER_BEGIN_FUNC_NAME(ITERABLE)(ITERABLE* const self)
#define ITER_BEGIN_FUNC_IMPL(ITERABLE)\
    ITER_BEGIN_FUNC_SIGNATURE(ITERABLE)

#define iter_begin(SELF)           (SELF)->iter->begin(SELF)

/* ********* iter_end *********** */
#define ITER_END_FUNC_NAME(ITERABLE)       CAT(Iterator(ITERABLE), _end)
#define ITER_END_FUNC_SIGNATURE(ITERABLE)  IterItem(ITERABLE)* ITER_END_FUNC_NAME(ITERABLE)(ITERABLE* const self)
#define ITER_END_FUNC_IMPL(ITERABLE)\
    ITER_END_FUNC_SIGNATURE(ITERABLE)

#define iter_end(SELF)           (SELF)->iter->end(SELF)

/* ********* iter_prev *********** */
#define ITER_PREV_FUNC_NAME(ITERABLE)       CAT(Iterator(ITERABLE), _prev)
#define ITER_PREV_FUNC_SIGNATURE(ITERABLE)  IterItem(ITERABLE)* ITER_PREV_FUNC_NAME(ITERABLE)(ITERABLE* const self)
#define ITER_PREV_FUNC_IMPL(ITERABLE)\
    ITER_PREV_FUNC_SIGNATURE(ITERABLE)

#define iter_prev(SELF)           (SELF)->iter->prev(SELF)

/* ********* iter_next *********** */
#define ITER_NEXT_FUNC_NAME(ITERABLE)       CAT(Iterator(ITERABLE), _next)
#define ITER_NEXT_FUNC_SIGNATURE(ITERABLE)  IterItem(ITERABLE)* ITER_NEXT_FUNC_NAME(ITERABLE)(ITERABLE* const self)
#define ITER_NEXT_FUNC_IMPL(ITERABLE)\
    ITER_NEXT_FUNC_SIGNATURE(ITERABLE)

#define iter_next(SELF)           (SELF)->iter->next(SELF)


#define ITER_DEFS(ITERABLE, TYPE)\
    typedef TYPE IterItem(ITERABLE);\
    ITER_DRIVER_DEF(ITERABLE);\

#define ITER_IMPL(ITERABLE, CURR_IMPL, BEGIN_IMPL, END_IMPL, PREV_IMPL, NEXT_IMPL)\
    ITER_CURR_FUNC_IMPL(ITERABLE)  CURR_IMPL\
    ITER_BEGIN_FUNC_IMPL(ITERABLE) BEGIN_IMPL\
    ITER_END_FUNC_IMPL(ITERABLE)   END_IMPL\
    ITER_PREV_FUNC_IMPL(ITERABLE)  PREV_IMPL\
    ITER_NEXT_FUNC_IMPL(ITERABLE)  NEXT_IMPL\


#define EACH(ITERABLE, EL, IT) \
    IterItem(ITERABLE)* EL = iter_begin(IT); EL != NULL; EL = iter_next(IT)

#define EACH_REV(ITERABLE, EL, IT) \
    IterItem(ITERABLE)* EL = iter_end(IT); EL != NULL; EL = iter_prev(IT)


#endif