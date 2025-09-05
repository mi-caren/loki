#ifndef RESULT_H
#define RESULT_H

#include "utils.h"
#include "generics.h"

/* *********** RESULT *********** */

typedef unsigned int Error;

#define Result(TYPE) GenericName(TYPE, Result)

#define RESULT_STRUCT_DEF(TYPE)\
    typedef struct {\
        IF_VOID(TYPE)(\
            ,\
            TYPE val;\
        )\
        Error err;\
        char* (*strerror) (Error err);\
    } Result(TYPE)

#define RES_OK 0

#define INIT_RESULT(TYPE)\
    {\
        .err = RES_OK,\
        .strerror = RES_STRERROR_FUNC_NAME(TYPE)\
    }

/* ********* OK *********** */
#define OK_FUNC_NAME(TYPE)              CAT(Result(TYPE), _ok)
#define OK_FUNC_SIGNATURE(TYPE) \
    Result(TYPE) OK_FUNC_NAME(TYPE)( \
        IF_VOID(TYPE)( \
            , \
            TYPE val \
        ) \
    )
#define OK_FUNC_IMPL(TYPE) \
    OK_FUNC_SIGNATURE(TYPE) { \
        Result(TYPE) res = INIT_RESULT(TYPE); \
        IF_VOID(TYPE) ( , res.val = val;) \
        return res; \
    }
#define Ok(TYPE, ...)       OK_FUNC_NAME(TYPE)(IF_VOID(TYPE)( , __VA_ARGS__))

/* ********* ERR *********** */
#define ERR_FUNC_NAME(TYPE)           CAT(Result(TYPE), _err)
#define ERR_FUNC_SIGNATURE(TYPE)      Result(TYPE) ERR_FUNC_NAME(TYPE)(Error code)
#define ERR_FUNC_IMPL(TYPE) \
    ERR_FUNC_SIGNATURE(TYPE) { \
        Result(TYPE) res; \
        res.err = code; \
        return res; \
    }
#define Err(TYPE, CODE)     ERR_FUNC_NAME(TYPE)(CODE)

/* ********* PANICK *********** */
#define RES_PANIC_FUNC_NAME(TYPE)           CAT(Result(TYPE), _panick)
#define RES_PANICK_FUNC_SIGNATURE(TYPE)     void RES_PANIC_FUNC_NAME(TYPE)(Result(TYPE) res, char* filename, unsigned int linenumber)
#define RES_PANICK_FUNC_IMPL(TYPE)\
    RES_PANICK_FUNC_SIGNATURE(TYPE) {\
        fprintf(STDERR_FILENO, "Panicked at %s:%d\n", filename, linenumber);\
        fprintf(STDERR_FILENO, "CODE: %d\n", res.err);\
        fprintf(STDERR_FILENO, "MESSAGE: %s\n", res.strerror(res.code));\
        abort();\
    }
#define res_panick(TYPE, RES, FILENAME, LINUNUMBER) RES_PANIC_FUNC_NAME(TYPE)(RES, FILENAME, LINUNUMBER)

/* ********* UNWRAP *********** */
#define UNWRAP_FUNC_NAME(TYPE)          CAT(Result(TYPE), _unwrap)
#define UNWRAP_FUNC_SIGNATURE(TYPE)     TYPE UNWRAP_FUNC_NAME(TYPE)(Result(TYPE) res, char* filename, int linenumber)
#define UNWRAP_FUNC_IMPL(TYPE) \
    UNWRAP_FUNC_SIGNATURE(TYPE) { \
        if (res.err != RES_OK) { \
            res_panic(TYPE, res, filename, linenumber);\
        } \
        IF_VOID(TYPE)( \
            , \
            return res.val; \
        ) \
    }
#define unwrap(TYPE, RES)            UNWRAP_FUNC_NAME(TYPE)(RES, __FILE__, __LINE__)

/* ********* TRY *********** */
#define TRY_FUNC_NAME(TYPE)             CAT(Result(TYPE), _try)
#define TRY_FUNC_SIGNATURE(TYPE)        TYPE TRY_FUNC_NAME(TYPE)(Result(TYPE) res)
#define TRY_FUNC_IMPL(TYPE) \
    TRY_FUNC_SIGNATURE(TYPE) { \
        _res_set_try_error(res.err); \
        IF_VOID(TYPE)( \
            , \
            return res.val; \
        ) \
    }
#define try(TYPE, EXPR) \
    TRY_FUNC_NAME(TYPE)(EXPR); \
    if (_res_get_try_error() != RES_OK) \
        return (Result(TYPE)){\
            .err = __get_try_error__(),\
            .strerror = RESULT_STRERROR_FUNC_NAME(TYPE)\
        };

/* ********* CATCH *********** */
#define catch(TYPE, EXPR, ERR) \
    TRY_FUNC_NAME(TYPE)(EXPR); \
    Error ERR = _res_get_try_error(); \
    if (ERR != RES_OK)

/* ********* PERROR *********** */
#define res_perror(RES, MSG)    fprintf(STDERR_FILENO, "%s: %s", MSG, RES.strerror(RES.code))

#define RES_STRERROR_FUNC_NAME(TYPE)        CAT(Result(TYPE), _strerror)
#define RES_STRERROR_FUNC_SIGNATURE(TYPE)   char* RES_STRERROR_FUNC_NAME(TYPE)(Error err);
#define RES_STRERROR_FUNC_IMPL(TYPE)\
        RES_STRERROR_FUNC_SIGNATURE(TYPE)

#define is_ok(RESULT)                   (RESULT.err == RES_OK)
#define is_err(RESULT)                  (RESULT.err != RES_OK)

void _res_set_try_error(Error err);
Error _res_get_try_error();

#define RESULT_DEFS(TYPE)\
    RESULT_STRUCT_DEF(TYPE);\
    OK_FUNC_SIGNATURE(TYPE);\
    ERR_FUNC_SIGNATURE(TYPE);\
    RES_PANICK_FUNC_SIGNATURE(TYPE);\
    UNWRAP_FUNC_SIGNATURE(TYPE);\
    TRY_FUNC_SIGNATURE(TYPE);\

#define RESULT_IMPL(TYPE, STRERROR_IMPL)\
    OK_FUNC_IMPL(TYPE)\
    ERR_FUNC_IMPL(TYPE)\
    RES_PANICK_FUNC_IMPL(TYPE)\
    UNWRAP_FUNC_IMPL(TYPE)\
    TRY_FUNC_IMPL(TYPE)\
    RES_STRERROR_FUNC_IMPL(TYPE) STRERROR_IMPL\


RESULT_DEFS(void)
RESULT_DEFS(int)
RESULT_DEFS(unsigned int)
RESULT_DEFS(char)


#endif