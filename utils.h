#ifndef UTILS_H
#define UTILS_H



/* *********** ERROR *********** */

#define RESULT(TYPE)                    Result_##TYPE

#define RESULT_STRUCT_DEF(TYPE)         typedef struct RESULT(TYPE) { TYPE val; Error err; } RESULT(TYPE)
#define RESULT_STRUCT_DEF_VOID          typedef struct RESULT(void) { Error err; } RESULT(void)

#define INIT_RESULT(TYPE)               { .val = INIT_RESULT_VAL_##TYPE , .err = INIT_RESULT_ERR }
#define INIT_RESULT_VOID                { .err = INIT_RESULT_ERR }

#define INIT_RESULT_ERR                 { .code = 0, .message = NULL }

#define INIT_RESULT_VAL_int             0



#define UNWRAP_FUNC_NAME(TYPE)          unwrap_##TYPE
#define UNWRAP_FUNC_SIGNATURE(TYPE)     TYPE UNWRAP_FUNC_NAME(TYPE)(RESULT(TYPE) result)

#define UNWRAP_FUNC_DEF(TYPE)           UNWRAP_FUNC_SIGNATURE(TYPE) { \
    if (result.err.code != 0) { \
        die(result.err); \
    } \
    return result.val; \
}

#define UNWRAP_FUNC_DEF_VOID            UNWRAP_FUNC_SIGNATURE(void) { \
    if (result.err.code) { \
        die(result.err); \
    } \
}


#define UNWRAP(RESULT, TYPE)            UNWRAP_FUNC_NAME(TYPE)(RESULT)



#define ERROR(RES, CODE, MSG) { \
    RES.err.code = CODE; \
    RES.err.message = MSG; \
    return RES; \
}

#define RETVAL(RES, VAL) { \
    RES.val = VAL; \
    return RES; \
}


typedef struct Error {
    unsigned int code;
    char *message;
} Error;

RESULT_STRUCT_DEF_VOID;
RESULT_STRUCT_DEF(int);


UNWRAP_FUNC_SIGNATURE(void);
UNWRAP_FUNC_SIGNATURE(int);



/* *********** DYNAMIC BUFFER *********** */

struct DynamicBuffer {
    char *b;
    int len;
};

#define DBUF_INIT {NULL, 0};



void dbuf_append(struct DynamicBuffer *dbuf, const char *s, int len);
void dbuf_free(struct DynamicBuffer *dbuf);



#endif
