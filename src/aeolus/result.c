#include <stdio.h>
#include <threads.h>
#include <unistd.h>
#include <stdlib.h>

#include "result.h"

static thread_local Error _res_try_error_code = RES_OK;
static thread_local char* (*_res_try_error_strerror)(Error);


void _res_set_try_error(Error code, char* (*strerror) (Error)) {
    _res_try_error_code = code;
    _res_try_error_strerror = strerror;
}

Error _res_get_try_error_code() {
    return _res_try_error_code;
}

char* (*_res_get_try_error_strerror())(Error) {
    return _res_try_error_strerror;
}

RESULT_IMPL(void)
RESULT_IMPL(int)
RESULT_IMPL(unsigned int)
RESULT_IMPL(char)
