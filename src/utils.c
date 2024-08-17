#include <string.h>
#include <stdlib.h>

#include "utils.h"


extern void die_error(Error err);


UNWRAP_FUNC_DEF(void)
UNWRAP_FUNC_DEF(int)
UNWRAP_FUNC_DEF(unsigned int)



void dbuf_append(struct DynamicBuffer *dbuf, const char *s, int len) {
    char *new = realloc(dbuf->b, dbuf->len + len);

    if (new == NULL) {
        die("dbuf_append/realloc");
    }
    memcpy(&new[dbuf->len], s, len);
    dbuf->b = new;
    dbuf->len += len;
}

void dbuf_free(struct DynamicBuffer *dbuf) {
    free(dbuf->b);
}


unsigned int saturating_sub(unsigned int n1, unsigned int n2) {
    int sub = n1 - n2;
    if (sub < 0) return 0;
    return (unsigned int)sub;
}

Direction editorKeyToDirection(enum EditorKey key) {
    switch (key) {
        case CTRL_ARROW_UP:
        case ARROW_UP:
            return Up;
        case CTRL_ARROW_DOWN:
        case ARROW_DOWN:
            return Down;
        case CTRL_ARROW_LEFT:
        case ARROW_LEFT:
            return Left;
        case CTRL_ARROW_RIGHT:
        case ARROW_RIGHT:
            return Right;
        default:
            die("editor/editorKeyToDirection");
            return 0;    // UNREACHABLE
    }
}
