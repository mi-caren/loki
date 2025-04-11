#ifndef RESULT_H
#define RESULT_H

#include "utils/utils.h"

/* *********** RESULT *********** */

#define IS_EMPTY(ARGS)                      APPEND_0_AND_TAKE_SECOND(PRIMITIVE_CAT(CAT_EMPTY_, ARGS))
#define CAT_EMPTY_                          ~, 1,

#define IS_UNSIGNED(TYPE)                   APPEND_0_AND_TAKE_SECOND(CAT(CAT_UNSIGNED_, TYPE))
#define IS_STRUCT(TYPE)                     APPEND_0_AND_TAKE_SECOND(CAT(CAT_STRUCT_, TYPE))
#define IS_VOID(TYPE)                       APPEND_0_AND_TAKE_SECOND(CAT(CAT_VOID_, TYPE))
#define CAT_UNSIGNED_unsigned               ~, 1,
#define CAT_STRUCT_struct                   ~, 1,
#define CAT_VOID_void                       ~, 1,

#define IF_UNSIGNED(TYPE)                   CAT(IF_, IS_UNSIGNED(TYPE))
#define IF_STRUCT(TYPE)                     CAT(IF_, IS_STRUCT(TYPE))
#define IF_VOID(TYPE)                       CAT(IF_, IS_VOID(TYPE))
#define IF_EMPTY(ARGS)                      CAT(IF_, IS_EMPTY(ARGS))



typedef int Error;

#define RESULT(TYPE)\
    IF_UNSIGNED(TYPE)(\
        CAT(RESULT_, PAREN_CLOSE(TYPE)),\
        IF_STRUCT(TYPE)(\
            CAT(RESULT_, PAREN_CLOSE(TYPE)),\
            CAT(result_, TYPE)\
        )\
    )


#define RESULT_unsigned                     RESULT_UNSIGNED(
#define RESULT_UNSIGNED(TYPE)               PRIMITIVE_CAT(result_unsigned_, TYPE)

#define RESULT_struct                       RESULT_STRUCT(
#define RESULT_STRUCT(NAME)                 PRIMITIVE_CAT(result_struct_, NAME)



#define RESULT_STRUCT_DEF(TYPE)\
    typedef struct RESULT(TYPE) {\
        IF_VOID(TYPE)(\
            ,\
            TYPE val;\
        )\
        Error err;\
    } RESULT(TYPE)



#define INIT_RESULT                     { .err = OK_CODE }


#define UNWRAP_FUNC_NAME(TYPE)          CAT(unwrap_, RESULT(TYPE))
#define UNWRAP_FUNC_SIGNATURE(TYPE)     TYPE UNWRAP_FUNC_NAME(TYPE)(RESULT(TYPE) result, char* filename, int linenumber)

#define UNWRAP_FUNC_DEF(TYPE) \
    UNWRAP_FUNC_SIGNATURE(TYPE) { \
        if (result.err != OK_CODE) { \
            char __err_msg_buf__[24]; \
            snprintf(__err_msg_buf__, 24, "ERROR CODE: [%d]", result.err); \
            panic(filename, linenumber, __err_msg_buf__); \
        } \
        IF_VOID(TYPE)( \
            , \
            return result.val; \
        ) \
    }


#define UNWRAP(TYPE, RESULT)            UNWRAP_FUNC_NAME(TYPE)(RESULT, __FILE__, __LINE__)

void __set_try_error__(Error err);
Error __get_try_error__();

#define TRY_FUNC_NAME(TYPE)             CAT(try_, RESULT(TYPE))
#define TRY_FUNC_SIGNATURE(TYPE)              TYPE TRY_FUNC_NAME(TYPE)(RESULT(TYPE) result)

#define TRY_FUNC_DEF(TYPE) \
    TRY_FUNC_SIGNATURE(TYPE) { \
        __set_try_error__(result.err); \
        IF_VOID(TYPE)( \
            , \
            return result.val; \
        ) \
    }

#define TRY(TYPE, EXPR) \
    TRY_FUNC_NAME(TYPE)(EXPR); \
    if (__get_try_error__() != OK_CODE) \
        return (RESULT(TYPE)){ .err = __get_try_error__() };


#define CATCH(TYPE, EXPR, ERR) \
    TRY_FUNC_NAME(TYPE)(EXPR); \
    ERR = __get_try_error__(); \
    if (ERR.code != OK_CODE) \



#define ERROR_FUNC_NAME(TYPE)           CAT(error_, RESULT(TYPE))
#define ERROR_FUNC_SIGNATURE(TYPE)      RESULT(TYPE) ERROR_FUNC_NAME(TYPE)(Error code)

#define ERROR_FUNC_DEF(TYPE) \
    inline ERROR_FUNC_SIGNATURE(TYPE) { \
        RESULT(TYPE) res; \
        res.err = code; \
        return res; \
    }

#define ERROR_PARAMS(CODE, MSG)         CODE, MSG
#define ERROR(TYPE, ...)                ERROR_FUNC_NAME(TYPE)(__VA_ARGS__)

#define OK_FUNC_NAME(TYPE)              CAT(ok_, RESULT(TYPE))
#define OK_FUNC_SIGNATURE(TYPE) \
    RESULT(TYPE) OK_FUNC_NAME(TYPE)( \
        IF_VOID(TYPE)( \
            , \
            TYPE val \
        ) \
    )

#define OK_FUNC_DEF(TYPE) \
    inline OK_FUNC_SIGNATURE(TYPE) { \
        RESULT(TYPE) res = INIT_RESULT; \
        IF_VOID(TYPE) ( , res.val = val;) \
        return res; \
    }

#define OK_CODE 0
#define OK(TYPE, ...)                   OK_FUNC_NAME(TYPE)(IF_VOID(TYPE)( , __VA_ARGS__))

#define IS_OK(RESULT)                   (RESULT.err == OK_CODE)
#define IS_ERROR(RESULT)                (RESULT.err != OK_CODE)


RESULT_STRUCT_DEF(void);
RESULT_STRUCT_DEF(int);
RESULT_STRUCT_DEF(unsigned int);


UNWRAP_FUNC_SIGNATURE(void);
UNWRAP_FUNC_SIGNATURE(int);
UNWRAP_FUNC_SIGNATURE(unsigned int);


TRY_FUNC_SIGNATURE(void);


ERROR_FUNC_SIGNATURE(void);

OK_FUNC_SIGNATURE(void);

#endif