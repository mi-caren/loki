#include <stdio.h>
#include <threads.h>
#include <unistd.h>

#include "result.h"

static thread_local Error _res_try_error = RES_OK;

inline void _res_set_try_error(Error err) {
    _res_try_error = err;
}
inline Error _res_get_try_error() {
    return _res_try_error;
}

RESULT_IMPL(void, )
RESULT_IMPL(int)
RESULT_IMPL(unsigned int)
RESULT_IMPL(char)
