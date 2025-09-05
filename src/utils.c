#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "aeolus/result.h"

static Error __try_error__ = OK_CODE;


unsigned int saturating_sub(unsigned int n1, unsigned int n2) {
    if (n2 > n1) return 0;
    return n1 -n2;
}

inline unsigned int umin(unsigned int a, unsigned int b) {
    return a < b ? a : b;
}

inline unsigned int umax(unsigned int a, unsigned int b) {
    return a > b ? a : b;
}


static void (*beforePanicFunc) () = NULL;


void atPanic(void (*beforePanicCallback) ()) {
    beforePanicFunc = beforePanicCallback;
}

// Calls a function registered with atPanic,
// print an error message and exits with error.
void panic(char* filename, int linenumber, char* msg) {
    if (beforePanicFunc) {
        beforePanicFunc();
    }

    fprintf(stderr, "Panicked at %s:%d\n", filename, linenumber);
    if (msg) {
        fprintf(stderr, "%s\n", msg);
    }
    exit(EXIT_FAILURE);
}


inline void __set_try_error__(Error err) {
    __try_error__ = err;
}
inline Error __get_try_error__() {
    return __try_error__;
}