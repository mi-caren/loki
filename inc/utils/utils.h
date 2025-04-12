#ifndef UTILS_H
#define UTILS_H

#include <unistd.h>

#define EMPTY()
#define DEFER(A)                            A EMPTY()
#define EXPAND(...)                         __VA_ARGS__
#define PRIMITIVE_CAT(A, ...)               A##__VA_ARGS__
#define CAT(A, B)                           PRIMITIVE_CAT(A, B)
#define PAREN_CLOSE(A)                      A)
#define APPEND_0_AND_TAKE_SECOND(...)       SECOND_ARG(__VA_ARGS__, 0)
#define SECOND_ARG(A, B, ...)               B

#define IF_1(A, B)                          A
#define IF_0(A, B)                          B


#define PANIC_FMT(FMT_MSG, ...) \
{ \
    char __err_msg_buf__[128]; \
    snprintf(__err_msg_buf__, 128, FMT_MSG, __VA_ARGS__); \
    panic(__FILE__, __LINE__, __err_msg_buf__); \
}
#define PANIC()

void panic(char* filename, int linenumber, char* msg);
void atPanic(void (*beforePanicCallback) ());

unsigned int umin(unsigned int a, unsigned int b);
unsigned int umax(unsigned int a, unsigned int b);

unsigned int saturating_sub(unsigned int n1, unsigned int n2);


#endif
