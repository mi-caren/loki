#ifndef UTILS_H
#define UTILS_H

#include <bits/types/struct_iovec.h>
#include <string.h>


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
            die_error(result.err);\
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


/* *********** ARRAY ******************** */
#define CAPITAL(TYPE)   CAT(CAPITAL_, TYPE)
#define CAPITAL_int     Int
#define CAPITAL_void    Void

#define ARRAY_NAME(TYPE)            CAT(Array, CAPITAL(TYPE))
#define ARRAY_NAME_UNSIGNED(TYPE)   CAT(ArrayUnsigned, CAPITAL(TYPE))

#define ARRAY_MEMBERS(TYPE)\
    TYPE* ptr;\
    size_t cur;\
    size_t len;\
    size_t cap;

#define DEF_ARRAY(TYPE)\
    typedef struct {\
        ARRAY_MEMBERS(TYPE)\
    } ARRAY_NAME(TYPE)

#define DEF_ARRAY_UNSIGNED(TYPE)\
    typedef struct {\
        ARRAY_MEMBERS(unsigned TYPE)\
    } ARRAY_NAME_UNSIGNED(TYPE)

#define ARRAY_PUSH_SIG(TYPE)\
    ARRAY_NAME(TYPE)* CAT(arrayPush, CAPITAL(TYPE))(ARRAY_NAME(TYPE)* a, TYPE el)
#define DEF_ARRAY_PUSH(TYPE)\
    ARRAY_PUSH_SIG(TYPE) {\
        if (!arrayIncSize((ArrayVoid*)a, sizeof(TYPE))) return NULL;\
        TYPE* ptr = a->ptr;\
        ptr[a->len++] = el;\
        return a;\
    }

#define ARRAY_PUSH_UNSIGNED_SIG(TYPE)\
    ARRAY_NAME_UNSIGNED(TYPE)* CAT(arrayPushUnsigned, CAPITAL(TYPE))(ARRAY_NAME_UNSIGNED(TYPE)* a, unsigned TYPE el)
#define DEF_ARRAY_PUSH_UNSIGNED(TYPE)\
    ARRAY_PUSH_UNSIGNED_SIG(TYPE) {\
        if (!arrayIncSize((ArrayVoid*)a, sizeof(unsigned TYPE))) return NULL;\
        unsigned TYPE* ptr = a->ptr;\
        ptr[a->len++] = el;\
        return a;\
    }
#define ARRAY_START_SIG\
    void arrayStart(ArrayVoid* a)
#define DEF_ARRAY_START\
    ARRAY_START_SIG {\
        a->cur = 0;\
    }
#define ARRAY_START(A)  arrayStart((ArrayVoid*)A)

#define ARRAY_END_SIG\
    void arrayEnd(ArrayVoid* a)
#define DEF_ARRAY_END\
    ARRAY_END_SIG {\
        a->cur = a->len;\
    }
#define ARRAY_END(A)  arrayEnd((ArrayVoid*)A)

#define ARRAY_NEXT_UNSIGNED_SIG(TYPE)\
    unsigned TYPE* CAT(arrayNextUnsigned, CAPITAL(TYPE))(ARRAY_NAME_UNSIGNED(TYPE)* a)
#define DEF_ARRAY_NEXT_UNSIGNED(TYPE)\
    ARRAY_NEXT_UNSIGNED_SIG(TYPE) {\
        if (a->cur == a->len) return NULL;\
        return &a->ptr[a->cur++];\
    }

#define ARRAY_PREV_UNSIGNED_SIG(TYPE)\
    unsigned TYPE* CAT(arrayPrevUnsigned, CAPITAL(TYPE))(ARRAY_NAME_UNSIGNED(TYPE)* a)
#define DEF_ARRAY_PREV_UNSIGNED(TYPE)\
    ARRAY_PREV_UNSIGNED_SIG(TYPE) {\
        if (a->cur == 0) return NULL;\
        return &a->ptr[--a->cur];\
    }

/* A is a reference to an array
   the loop will provide a reference to the current element
   accessible via the variable cur, which is a pointer to it*/
#define ARRAY_FOR_EACH_UINT(A)\
    ARRAY_START(A);\
    unsigned int* cur = NULL;\
    while((cur = arrayNextUnsignedInt(A)) != NULL)

#define ARRAY_FOR_EACH_UINT_REV(A)\
    ARRAY_END(A);\
    unsigned int* cur = NULL;\
    while((cur = arrayPrevUnsignedInt(A)) != NULL)

#define ARRAY_EMPTY_SIG\
    void arrayEmpty(ArrayVoid* a)
#define DEF_ARRAY_EMPTY\
    ARRAY_EMPTY_SIG {\
        a->cur = 0;\
        a->len = 0;\
    }
#define ARRAY_EMPTY(A)  arrayEmpty((ArrayVoid*)A)

#define ARRAY_FREE_SIG\
    void arrayFree(ArrayVoid* a)
#define DEF_ARRAY_FREE\
    ARRAY_FREE_SIG {\
        arrayEmpty(a);\
        free(a->ptr);\
        a->ptr = NULL;\
        a->cap = 0;\
    }
#define ARRAY_FREE(A)   arrayFree((ArrayVoid*)A)

#define ARRAY_NEW(ARRAY)   (ARRAY){ NULL, 0, 0, 0 }

DEF_ARRAY(void);
DEF_ARRAY(int);
DEF_ARRAY_UNSIGNED(int);

ARRAY_PUSH_SIG(int);
ARRAY_PUSH_UNSIGNED_SIG(int);

ARRAY_START_SIG;
ARRAY_END_SIG;
ARRAY_NEXT_UNSIGNED_SIG(int);
ARRAY_PREV_UNSIGNED_SIG(int);

ARRAY_EMPTY_SIG;

ARRAY_FREE_SIG;

/* *********** DYNAMIC BUFFER *********** */

extern void die(const char *s);

struct DynamicBuffer {
    char *b;
    int len;
};

#define DBUF_INIT {NULL, 0};
#define TAB_SPACE_NUM    4
/*
    Stop Chars are those characters that delimits words:
    spaces, null chars.
*/
#define CHAR_IS_STOPCHAR(C)        ( C == ' ' || C == '.' || C == '"' || C == '\'' || C == '(' || C == '[' || C == '{' )

#define CTRL_KEY(k)      ((k) & 0x1f)
#define COLOR_SEQ_SIZE 10

#define SHIFT_KEY(c)\
(\
    c == SHIFT_ARROW_UP\
    || c == SHIFT_ARROW_DOWN\
    || c == SHIFT_ARROW_LEFT\
    || c == SHIFT_ARROW_RIGHT\
    || c == CTRL_SHIFT_ARROW_LEFT\
    || c == CTRL_SHIFT_ARROW_RIGHT\
)

typedef enum {
    Up,
    Down,
    Left,
    Right,
}  Direction;

enum EditorKey {
    BACKSPACE = 127,
    ARROW_LEFT = 1000,    // avoid conflicts with regular chars
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN,

    CTRL_ARROW_UP,
    CTRL_ARROW_DOWN,
    CTRL_ARROW_RIGHT,
    CTRL_ARROW_LEFT,

    SHIFT_ARROW_UP,
    SHIFT_ARROW_DOWN,
    SHIFT_ARROW_RIGHT,
    SHIFT_ARROW_LEFT,

    CTRL_SHIFT_ARROW_RIGHT,
    CTRL_SHIFT_ARROW_LEFT,
};

void dbuf_append(struct DynamicBuffer *dbuf, const char *s, int len);
void dbuf_free(struct DynamicBuffer *dbuf);

unsigned int saturating_sub(unsigned int n1, unsigned int n2);

Direction editorKeyToDirection(enum EditorKey key);


#endif
