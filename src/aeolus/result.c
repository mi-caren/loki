#include <stdio.h>
#include <threads.h>
#include <unistd.h>
#include <stdlib.h>

#include "result.h"

static thread_local bool _res_try_err = false;
static thread_local Err _res_try_strerror = NULL;


void _res_set_try_error(bool err, Err strerror) {
    _res_try_err = err;
    if (err)
        _res_try_strerror = strerror;
    else
        _res_try_strerror = NULL;
}

bool _res_get_try_err() {
    return _res_try_err;
}

Err _res_get_try_strerror() {
    return _res_try_strerror;
}

RESULT_IMPL(void)
RESULT_IMPL(int)
RESULT_IMPL(unsigned int)
RESULT_IMPL(char)
