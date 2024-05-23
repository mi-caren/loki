#ifndef UTILS_H
#define UTILS_H



#define PRIMITIVE_CAT(A, ...)               A##__VA_ARGS__
#define CAT(A, B)                           PRIMITIVE_CAT(A, B)
#define PAREN_CLOSE(A)                      A)
#define APPEND_0_AND_TAKE_SECOND(...)       SECOND_ARG(__VA_ARGS__, 0)
#define SECOND_ARG(A, B, ...)               B

#define IF_1(A, B)                          A
#define IF_0(A, B)                          B


/* *********** RESULT *********** */

#define IS_UNSIGNED(TYPE)                   APPEND_0_AND_TAKE_SECOND(CAT(CAT_UNSIGNED_, TYPE))
#define IS_STRUCT(TYPE)                     APPEND_0_AND_TAKE_SECOND(CAT(CAT_STRUCT_, TYPE))
#define IS_VOID(TYPE)                       APPEND_0_AND_TAKE_SECOND(CAT(CAT_VOID_, TYPE))
#define CAT_UNSIGNED_unsigned               ~, 1,
#define CAT_STRUCT_struct                   ~, 1,
#define CAT_VOID_void                       ~, 1,

#define IF_UNSIGNED(TYPE)                   CAT(IF_, IS_UNSIGNED(TYPE))
#define IF_STRUCT(TYPE)                     CAT(IF_, IS_STRUCT(TYPE))
#define IF_VOID(TYPE)                       CAT(IF_, IS_VOID(TYPE))



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



#define INIT_RESULT { .err = INIT_RESULT_ERR }


#define INIT_RESULT_ERR                 { .code = 0, .message = NULL }



#define UNWRAP_FUNC_NAME(TYPE)          CAT(unwrap_, RESULT(TYPE))
#define UNWRAP_FUNC_SIGNATURE(TYPE)     TYPE UNWRAP_FUNC_NAME(TYPE)(RESULT(TYPE) result)

#define UNWRAP_FUNC_DEF(TYPE)\
    UNWRAP_FUNC_SIGNATURE(TYPE) {\
        if (result.err.code != 0) {\
            die(result.err);\
        }\
        IF_VOID(TYPE)(\
            ,\
            return result.val;\
        )\
    }


#define UNWRAP(RESULT, TYPE)            UNWRAP_FUNC_NAME(TYPE)(RESULT)



#define ERROR(RES, CODE, MSG)\
    {\
        RES.err.code = CODE;\
        RES.err.message = MSG;\
        return RES;\
    }

#define RETVAL(RES, VAL)\
    {\
        RES.val = VAL;\
        return RES;\
    }


typedef struct Error {
    unsigned int code;
    char *message;
} Error;

RESULT_STRUCT_DEF(void);
RESULT_STRUCT_DEF(int);
RESULT_STRUCT_DEF(unsigned int);


UNWRAP_FUNC_SIGNATURE(void);
UNWRAP_FUNC_SIGNATURE(int);
UNWRAP_FUNC_SIGNATURE(unsigned int);



/* *********** DYNAMIC BUFFER *********** */

struct DynamicBuffer {
    char *b;
    int len;
};

#define DBUF_INIT {NULL, 0};



RESULT(void) dbuf_append(struct DynamicBuffer *dbuf, const char *s, int len) __attribute__((warn_unused_result));
void dbuf_free(struct DynamicBuffer *dbuf);

unsigned int saturating_sub(unsigned int n1, unsigned int n2);


#endif
