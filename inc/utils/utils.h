#ifndef UTILS_H
#define UTILS_H

#include <bits/types/struct_iovec.h>


#define PRIMITIVE_CAT(A, ...)               A##__VA_ARGS__
#define CAT(A, B)                           PRIMITIVE_CAT(A, B)
#define PAREN_CLOSE(A)                      A)
#define APPEND_0_AND_TAKE_SECOND(...)       SECOND_ARG(__VA_ARGS__, 0)
#define SECOND_ARG(A, B, ...)               B

#define IF_1(A, B)                          A
#define IF_0(A, B)                          B

unsigned int umin(unsigned int a, unsigned int b);
unsigned int umax(unsigned int a, unsigned int b);

unsigned int saturating_sub(unsigned int n1, unsigned int n2);


#endif
