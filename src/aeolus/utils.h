#ifndef UTILS_H
#define UTILS_H

#include <unistd.h>

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

#define EMPTY()
#define DEFER(A)                            A EMPTY()
#define EXPAND(...)                         __VA_ARGS__
#define PRIMITIVE_CAT(A, ...)               A##__VA_ARGS__
#define CAT(A, ...)                         PRIMITIVE_CAT(A, __VA_ARGS__)
#define PAREN_CLOSE(A)                      A)
#define APPEND_0_AND_TAKE_SECOND(...)       SECOND_ARG(__VA_ARGS__, 0)
#define SECOND_ARG(A, B, ...)               B
#define CAT_COMMA(A, B)                     A, B

#define IF_1(A, B)                          A
#define IF_0(A, B)                          B


void atPanic(void (*beforePanicCallback) ());

unsigned int umin(unsigned int a, unsigned int b);
unsigned int umax(unsigned int a, unsigned int b);

unsigned int saturating_sub(unsigned int n1, unsigned int n2);


#endif
