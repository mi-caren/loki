#include <stdio.h>
#include <string.h>

#include "vec.h"

size_t vec_cap_from_size(size_t size) {
    if (size == 0) {
        size = 1;
    }

    size_t cap = 1;
    while (cap < size) {
        cap *= 2;
    }

    return cap;
}

VEC_IMPL(char)
VEC_IMPL(unsigned int)
