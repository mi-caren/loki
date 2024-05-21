#include <string.h>
#include <stdlib.h>

#include "utils.h"


extern void die(Error err);


UNWRAP_FUNC_DEF_VOID
UNWRAP_FUNC_DEF(int)



void dbuf_append(struct DynamicBuffer *dbuf, const char *s, int len) {
    char *new = realloc(dbuf->b, dbuf->len + len);

    if (new == NULL) return;
    memcpy(&new[dbuf->len], s, len);
    dbuf->b = new;
    dbuf->len += len;
}

void dbuf_free(struct DynamicBuffer *dbuf) {
    free(dbuf->b);
}

