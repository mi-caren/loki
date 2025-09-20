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
        Err err;\
        IF_VOID(TYPE)(\
            ,\
            TYPE val;\
        )\
    } Res(TYPE)

#define RES_OK 0

#ifdef NDEBUG
    #define errdbg(MSG, ERR, FILENAME, LINENUMBER)
#else
    #ifdef PRE_ERRDBG
        void pre_errdbg();
        #define _PRE_ERRDBG pre_errdbg();
    #else
        #define _PRE_ERRDBG
    #endif

    #ifdef POST_ERRDBG
        void post_errdbg();
        #define _POST_ERRDBG post_errdbg();
    #else
        #define _POST_ERRDBG
    #endif

    #define errdbg(MSG, ERR, FILENAME, LINENUMBER)\
        _PRE_ERRDBG\
        fprintf(stderr, "[errdbg] %s:%d\t %s: %s\n\r", FILENAME, LINENUMBER, MSG, ERR);\
        _POST_ERRDBG
#endif

/* ********* OK *********** */
#define ok(TYPE, ...)\
    (Res(TYPE)) {\
        .err = NULL,\
        IF_VOID(TYPE) ( , .val = __VA_ARGS__)\
    }

/* ********* ERR *********** */
/* ********* UNWRAP *********** */
#define ERR_FUNC_NAME(TYPE)          CAT(Res(TYPE), _err)
#define ERR_FUNC_SIGNATURE(TYPE)     Res(TYPE) ERR_FUNC_NAME(TYPE)(Err err, const char* filename, int linenumber)
#define ERR_FUNC_IMPL(TYPE) \
    ERR_FUNC_SIGNATURE(TYPE) { \
        errdbg("err", err, filename, linenumber)\
        return (Res(TYPE)) {\
            .err = err,\
        };\
    }
#define err(TYPE, ERR)  ERR_FUNC_NAME(TYPE)(ERR, __FILE__, __LINE__)

/* ********* PANIC *********** */
#define RES_PANIC_FUNC_NAME(TYPE)           CAT(Res(TYPE), _panic)
#define RES_PANIC_FUNC_SIGNATURE(TYPE)     void RES_PANIC_FUNC_NAME(TYPE)(Res(TYPE) res, const char* filename, unsigned int linenumber)
#define RES_PANIC_FUNC_IMPL(TYPE)\
    RES_PANIC_FUNC_SIGNATURE(TYPE) {\
        fprintf(stderr, "Panicked at %s:%d\n", filename, linenumber);\
        if (res.err) {\
            fprintf(stderr, "%s\n", res.err);\
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
            return res.val; \
        ) \
    }
#define unwrap(TYPE, RES)            UNWRAP_FUNC_NAME(TYPE)(RES, __FILE__, __LINE__)

/* ********* TRY *********** */
#define TRY_FUNC_NAME(TYPE)             CAT(Res(TYPE), _try)
#define TRY_FUNC_SIGNATURE(TYPE)        TYPE TRY_FUNC_NAME(TYPE)(Res(TYPE) res)
#define TRY_FUNC_IMPL(TYPE) \
    TRY_FUNC_SIGNATURE(TYPE) { \
        _res_set_try_err(res.err); \
        IF_VOID(TYPE)( \
            , \
            return res.val; \
        ) \
    }
#define try(TYPE, EXPR) \
    TRY_FUNC_NAME(TYPE)(EXPR); \
    if (_res_get_try_err()) {\
        errdbg("try", _res_get_try_err(), __FILE__, __LINE__)\
        return (Res(TYPE)) {\
            .err = _res_get_try_err(),\
        };\
    }

/* ********* CATCH *********** */
#define catch(TYPE, EXPR, ERR) \
    TRY_FUNC_NAME(TYPE)(EXPR); \
    Err ERR = _res_get_try_err();\
    if (ERR)

#define is_ok(RES)                   (RES.err == NULL)
#define is_err(RES)                  (RES.err != NULL)

void _res_set_try_err(Err err);
Err _res_get_try_err();

#define RESULT_DEFS(TYPE)\
    RESULT_STRUCT_DEF(TYPE);\
    ERR_FUNC_SIGNATURE(TYPE);\
    RES_PANIC_FUNC_SIGNATURE(TYPE);\
    UNWRAP_FUNC_SIGNATURE(TYPE);\
    TRY_FUNC_SIGNATURE(TYPE);\

#define RESULT_IMPL(TYPE)\
    RES_PANIC_FUNC_IMPL(TYPE)\
    ERR_FUNC_IMPL(TYPE)\
    UNWRAP_FUNC_IMPL(TYPE)\
    TRY_FUNC_IMPL(TYPE)\


RESULT_DEFS(void)
RESULT_DEFS(int)
RESULT_DEFS(unsigned int)
RESULT_DEFS(char)


#endif