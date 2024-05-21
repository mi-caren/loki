#include <string.h>
#include <stdlib.h>

#include "utils.h"


extern void die(Error err);


UNWRAP_FUNC_DEF_VOID
UNWRAP_FUNC_DEF(int)



RESULT(void) dbuf_append(struct DynamicBuffer *dbuf, const char *s, int len) {
    RESULT(void) res = INIT_RESULT_VOID;
    char *new = realloc(dbuf->b, dbuf->len + len);

    if (new == NULL) {
        ERROR(res, 1, "utils/dbuf_append/realloc");
    }
    memcpy(&new[dbuf->len], s, len);
    dbuf->b = new;
    dbuf->len += len;

    return res;
}

void dbuf_free(struct DynamicBuffer *dbuf) {
    free(dbuf->b);
}

