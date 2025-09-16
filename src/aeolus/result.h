#ifndef RESULT_H
#define RESULT_H

#include "utils.h"
#include "generics.h"
#include <string.h>

/* *********** RESULT *********** */

typedef unsigned int Error;

#define Res(TYPE) GenericName(TYPE, Result)

#define RESULT_STRUCT_DEF(TYPE)\
    typedef struct {\
        Error err;\
        union {\
            IF_VOID(TYPE)(\
                ,\
                TYPE val;\
            )\
            char* (*const strerror) (Error err);\
        } res;\
    } Res(TYPE)

#define RES_OK 0

/* ********* OK *********** */
#define OK_FUNC_NAME(TYPE)              CAT(Res(TYPE), _ok)
#define OK_FUNC_SIGNATURE(TYPE) \
    Res(TYPE) OK_FUNC_NAME(TYPE)( \
        IF_VOID(TYPE)( \
            , \
            TYPE val \
        ) \
    )
#define OK_FUNC_IMPL(TYPE) \
    OK_FUNC_SIGNATURE(TYPE) { \
        Res(TYPE) res = {\
            .err = RES_OK,\
            IF_VOID(TYPE) ( , .res.val = val) \
        }; \
        return res; \
    }
#define ok(TYPE, ...)       OK_FUNC_NAME(TYPE)(IF_VOID(TYPE)( , __VA_ARGS__))

/* ********* ERR *********** */
#define ERR_FUNC_NAME(TYPE)           CAT(Res(TYPE), _err)
#define ERR_FUNC_SIGNATURE(TYPE)      Res(TYPE) ERR_FUNC_NAME(TYPE)(Error code, char* (*const strerror) (Error err))
#define ERR_FUNC_IMPL(TYPE) \
    ERR_FUNC_SIGNATURE(TYPE) { \
        Res(TYPE) res = {\
            .err = code,\
            .res.strerror = strerror,\
        }; \
        return res; \
    }
#define err(TYPE, CODE, STRERROR)     ERR_FUNC_NAME(TYPE)(CODE, STRERROR)

/* ********* PANIC *********** */
#define RES_PANIC_FUNC_NAME(TYPE)           CAT(Res(TYPE), _panic)
#define RES_PANIC_FUNC_SIGNATURE(TYPE)     void RES_PANIC_FUNC_NAME(TYPE)(Res(TYPE) res, char* filename, unsigned int linenumber)
#define RES_PANIC_FUNC_IMPL(TYPE)\
    RES_PANIC_FUNC_SIGNATURE(TYPE) {\
        fprintf(stderr, "Panicked at %s:%d\n", filename, linenumber);\
        fprintf(stderr, "CODE: %d\n", res.err);\
        fprintf(stderr, "MESSAGE: %s\n", res.res.strerror(res.err));\
        abort();\
    }
#define res_panic(TYPE, RES, FILENAME, LINUNUMBER) RES_PANIC_FUNC_NAME(TYPE)(RES, FILENAME, LINUNUMBER)

/* ********* UNWRAP *********** */
#define UNWRAP_FUNC_NAME(TYPE)          CAT(Res(TYPE), _unwrap)
#define UNWRAP_FUNC_SIGNATURE(TYPE)     TYPE UNWRAP_FUNC_NAME(TYPE)(Res(TYPE) res, char* filename, int linenumber)
#define UNWRAP_FUNC_IMPL(TYPE) \
    UNWRAP_FUNC_SIGNATURE(TYPE) { \
        if (res.err != RES_OK) { \
            res_panic(TYPE, res, filename, linenumber);\
        } \
        IF_VOID(TYPE)( \
            , \
            return res.res.val; \
        ) \
    }
#define unwrap(TYPE, RES)            UNWRAP_FUNC_NAME(TYPE)(RES, __FILE__, __LINE__)

/* ********* TRY *********** */
#define TRY_FUNC_NAME(TYPE)             CAT(Res(TYPE), _try)
#define TRY_FUNC_SIGNATURE(TYPE)        TYPE TRY_FUNC_NAME(TYPE)(Res(TYPE) res)
#define TRY_FUNC_IMPL(TYPE) \
    TRY_FUNC_SIGNATURE(TYPE) { \
        _res_set_try_error(res.err, res.res.strerror); \
        IF_VOID(TYPE)( \
            , \
            return res.res.val; \
        ) \
    }
#define try(TYPE, EXPR) \
    TRY_FUNC_NAME(TYPE)(EXPR); \
    if (_res_get_try_error_code() != RES_OK) \
        return (Res(TYPE)){\
            .err = _res_get_try_error_code(),\
            .res.strerror = _res_get_try_error_strerror()\
        };

/* ********* CATCH *********** */
#define catch(TYPE, EXPR, ERR) \
    TRY_FUNC_NAME(TYPE)(EXPR); \
    Error ERR = _res_get_try_error_code(); \
    if (ERR != RES_OK)

/* ********* PERROR *********** */
#define res_perror(RES, MSG)    fprintf(STDERR_FILENO, "%s: %s", MSG, RES.res.strerror(RES.code))
/* 
#define RES_STRERROR_FUNC_NAME(TYPE)        CAT(Res(TYPE), _strerror)
#define RES_STRERROR_FUNC_SIGNATURE(TYPE)   char* RES_STRERROR_FUNC_NAME(TYPE)(Error err);
#define RES_STRERROR_FUNC_IMPL(TYPE)\
        RES_STRERROR_FUNC_SIGNATURE(TYPE)
 */
#define is_ok(RESULT)                   (RESULT.err == RES_OK)
#define is_err(RESULT)                  (RESULT.err != RES_OK)

void _res_set_try_error(Error code, char* (*const strerror) (Error err));
Error _res_get_try_error_code();
char* (*_res_get_try_error_strerror())(Error);

#define RESULT_DEFS(TYPE)\
    RESULT_STRUCT_DEF(TYPE);\
    OK_FUNC_SIGNATURE(TYPE);\
    ERR_FUNC_SIGNATURE(TYPE);\
    RES_PANIC_FUNC_SIGNATURE(TYPE);\
    UNWRAP_FUNC_SIGNATURE(TYPE);\
    TRY_FUNC_SIGNATURE(TYPE);\

#define RESULT_IMPL(TYPE)\
    OK_FUNC_IMPL(TYPE)\
    ERR_FUNC_IMPL(TYPE)\
    RES_PANIC_FUNC_IMPL(TYPE)\
    UNWRAP_FUNC_IMPL(TYPE)\
    TRY_FUNC_IMPL(TYPE)\
    /* RES_STRERROR_FUNC_IMPL(TYPE) STRERROR_IMPL */\


RESULT_DEFS(void)
RESULT_DEFS(int)
RESULT_DEFS(unsigned int)
RESULT_DEFS(char)


#endif