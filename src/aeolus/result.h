#ifndef RESULT_H
#define RESULT_H

#include "utils.h"
#include "generics.h"
#include <string.h>

/* *********** RESULT *********** */

typedef const char* Err;

#define Res(TYPE) GenericName(TYPE, Result)

#define RESULT_STRUCT_DEF(TYPE)\
    typedef struct {\
        bool err;\
        union {\
            IF_VOID(TYPE)(\
                ,\
                TYPE val;\
            )\
            Err strerror;\
        } res;\
    } Res(TYPE)

#define RES_OK 0

/* ********* OK *********** */
#define ok(TYPE, ...)\
    (Res(TYPE)) {\
        .err = false,\
        IF_VOID(TYPE) ( , .res.val = __VA_ARGS__)\
    }

/* ********* ERR *********** */
#define err(TYPE, ERR)\
    (Res(TYPE)) {\
        .err = true,\
        .res.strerror = ERR,\
    };

/* ********* PANIC *********** */
#define RES_PANIC_FUNC_NAME(TYPE)           CAT(Res(TYPE), _panic)
#define RES_PANIC_FUNC_SIGNATURE(TYPE)     void RES_PANIC_FUNC_NAME(TYPE)(Res(TYPE) res, const char* filename, unsigned int linenumber)
#define RES_PANIC_FUNC_IMPL(TYPE)\
    RES_PANIC_FUNC_SIGNATURE(TYPE) {\
        fprintf(stderr, "Panicked at %s:%d\n", filename, linenumber);\
        if (res.err) {\
            fprintf(stderr, "%s\n", res.res.strerror);\
        }\
        abort();\
    }
#define res_panic(TYPE, RES, FILENAME, LINUNUMBER) RES_PANIC_FUNC_NAME(TYPE)(RES, FILENAME, LINUNUMBER)

/* ********* UNWRAP *********** */
#define UNWRAP_FUNC_NAME(TYPE)          CAT(Res(TYPE), _unwrap)
#define UNWRAP_FUNC_SIGNATURE(TYPE)     TYPE UNWRAP_FUNC_NAME(TYPE)(Res(TYPE) res, const char* filename, int linenumber)
#define UNWRAP_FUNC_IMPL(TYPE) \
    UNWRAP_FUNC_SIGNATURE(TYPE) { \
        if (res.err) { \
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
    if (_res_get_try_err()) \
        return (Res(TYPE)){\
            .err = _res_get_try_err(),\
            .res.strerror = _res_get_try_strerror()\
        };

/* ********* CATCH *********** */
#define catch(TYPE, EXPR, ERR) \
    TRY_FUNC_NAME(TYPE)(EXPR); \
    Err ERR = _res_get_try_strerror();\
    if (ERR)

/* ********* PERROR *********** */
/* #define res_perror(RES, MSG)    fprintf(STDERR_FILENO, "%s: %s", MSG, RES.res.strerror(RES.code)) */
/* 
#define RES_STRERROR_FUNC_NAME(TYPE)        CAT(Res(TYPE), _strerror)
#define RES_STRERROR_FUNC_SIGNATURE(TYPE)   char* RES_STRERROR_FUNC_NAME(TYPE)(Error err);
#define RES_STRERROR_FUNC_IMPL(TYPE)\
        RES_STRERROR_FUNC_SIGNATURE(TYPE)
 */
#define is_ok(RES)                   (RES.err == false)
#define is_err(RES)                  (RES.err == true)

void _res_set_try_error(bool err, Err strerror);
bool _res_get_try_err();
Err _res_get_try_strerror();

#define RESULT_DEFS(TYPE)\
    RESULT_STRUCT_DEF(TYPE);\
    RES_PANIC_FUNC_SIGNATURE(TYPE);\
    UNWRAP_FUNC_SIGNATURE(TYPE);\
    TRY_FUNC_SIGNATURE(TYPE);\

#define RESULT_IMPL(TYPE)\
    RES_PANIC_FUNC_IMPL(TYPE)\
    UNWRAP_FUNC_IMPL(TYPE)\
    TRY_FUNC_IMPL(TYPE)\


RESULT_DEFS(void)
RESULT_DEFS(int)
RESULT_DEFS(unsigned int)
RESULT_DEFS(char)


#endif