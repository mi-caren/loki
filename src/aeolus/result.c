#include <stdio.h>
#include <threads.h>
#include <unistd.h>
#include <stdlib.h>

#include "result.h"

static thread_local Err _res_try_err = NULL;

void _res_set_try_err(Err err) {
    _res_try_err = err;
}

Err _res_get_try_err() {
    return _res_try_err;
}


RESULT_IMPL(void)
RESULT_IMPL(int)
RESULT_IMPL(unsigned int)
RESULT_IMPL(char)
