#ifndef RESULT_H
#define RESULT_H

#include "utils.h"
#include "generics.h"

/* *********** RESULT *********** */

typedef int Error;

#define Result(TYPE) GenericName(TYPE, Result)

#define RESULT_STRUCT_DEF(TYPE)\
    typedef struct {\
        IF_VOID(TYPE)(\
            ,\
            TYPE val;\
        )\
        Error err;\
    } Result(TYPE)



#define INIT_RESULT                     { .err = OK_CODE }


#define UNWRAP_FUNC_NAME(TYPE)          CAT(Result(TYPE), _unwrap)
#define UNWRAP_FUNC_SIGNATURE(TYPE)     TYPE UNWRAP_FUNC_NAME(TYPE)(Result(TYPE) res, char* filename, int linenumber)
#define UNWRAP_FUNC_IMPL(TYPE) \
    UNWRAP_FUNC_SIGNATURE(TYPE) { \
        if (res.err != OK_CODE) { \
            char __err_msg_buf__[24]; \
            snprintf(__err_msg_buf__, 24, "ERROR CODE: [%d]", res.err); \
            panic(filename, linenumber, __err_msg_buf__); \
        } \
        IF_VOID(TYPE)( \
            , \
            return res.val; \
        ) \
    }

#define unwrap(TYPE, RESULT)            UNWRAP_FUNC_NAME(TYPE)(RESULT, __FILE__, __LINE__)

void __set_try_error__(Error err);
Error __get_try_error__();

#define TRY_FUNC_NAME(TYPE)             CAT(Result(TYPE), _try)
#define TRY_FUNC_SIGNATURE(TYPE)        TYPE TRY_FUNC_NAME(TYPE)(Result(TYPE) res)
#define TRY_FUNC_IMPL(TYPE) \
    TRY_FUNC_SIGNATURE(TYPE) { \
        __set_try_error__(res.err); \
        IF_VOID(TYPE)( \
            , \
            return res.val; \
        ) \
    }

#define try(TYPE, EXPR) \
    TRY_FUNC_NAME(TYPE)(EXPR); \
    if (__get_try_error__() != OK_CODE) \
        return (Result(TYPE)){ .err = __get_try_error__() };


#define catch(TYPE, EXPR, ERR) \
    TRY_FUNC_NAME(TYPE)(EXPR); \
    Error ERR = __get_try_error__(); \
    if (ERR != OK_CODE)




#define ERR_FUNC_NAME(TYPE)           CAT(Result(TYPE), _err)
#define ERR_FUNC_SIGNATURE(TYPE)      Result(TYPE) ERR_FUNC_NAME(TYPE)(Error code)
#define ERR_FUNC_IMPL(TYPE) \
    inline ERR_FUNC_SIGNATURE(TYPE) { \
        Result(TYPE) res; \
        res.err = code; \
        return res; \
    }

#define ERR_PARAMS(CODE, MSG)         CODE, MSG
#define Err(TYPE, CODE)               ERR_FUNC_NAME(TYPE)(CODE)

#define OK_FUNC_NAME(TYPE)              CAT(Result(TYPE), _ok)
#define OK_FUNC_SIGNATURE(TYPE) \
    Result(TYPE) OK_FUNC_NAME(TYPE)( \
        IF_VOID(TYPE)( \
            , \
            TYPE val \
        ) \
    )
#define OK_FUNC_IMPL(TYPE) \
    inline OK_FUNC_SIGNATURE(TYPE) { \
        Result(TYPE) res = INIT_RESULT; \
        IF_VOID(TYPE) ( , res.val = val;) \
        return res; \
    }

#define OK_CODE 0
#define Ok(TYPE, ...)                   OK_FUNC_NAME(TYPE)(IF_VOID(TYPE)( , __VA_ARGS__))

#define is_ok(RESULT)                   (RESULT.err == OK_CODE)
#define is_err(RESULT)                  (RESULT.err != OK_CODE)

#define RESULT_DEFS(TYPE) \
    RESULT_STRUCT_DEF(TYPE); \
    UNWRAP_FUNC_SIGNATURE(TYPE); \
    TRY_FUNC_SIGNATURE(TYPE); \
    ERR_FUNC_SIGNATURE(TYPE); \
    OK_FUNC_SIGNATURE(TYPE);

#define RESULT_IMPL(TYPE) \
    UNWRAP_FUNC_IMPL(TYPE) \
    TRY_FUNC_IMPL(TYPE) \
    ERR_FUNC_IMPL(TYPE) \
    OK_FUNC_IMPL(TYPE)


RESULT_DEFS(void)
RESULT_DEFS(int)
RESULT_DEFS(unsigned int)
RESULT_DEFS(char)


#endif