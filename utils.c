#include <string.h>
#include <stdlib.h>

#include "utils.h"


extern void die(Error err);


UNWRAP_FUNC_DEF(void)
UNWRAP_FUNC_DEF(int)
UNWRAP_FUNC_DEF(unsigned int)



RESULT(void) dbuf_append(struct DynamicBuffer *dbuf, const char *s, int len) {
    RESULT(void) res = INIT_RESULT;
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


unsigned int saturating_sub(unsigned int n1, unsigned int n2) {
    int sub = n1 - n2;
    if (sub < 0) return 0;
    return (unsigned int)sub;
}
