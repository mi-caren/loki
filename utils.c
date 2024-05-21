#include <string.h>
#include <stdlib.h>

#include "utils.h"

void ab_append(struct DynamicBuffer *ab, const char *s, int len) {
    char *new = realloc(ab->b, ab->len + len);

    if (new == NULL) return;
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void ab_free(struct DynamicBuffer *ab) {
    free(ab->b);
}
