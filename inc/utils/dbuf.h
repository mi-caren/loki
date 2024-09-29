#ifndef DBUF_H
#define DBUF_H

/* *********** DYNAMIC BUFFER *********** */

extern void die(const char *s);

struct DynamicBuffer {
    char *b;
    int len;
};

#define DBUF_INIT {NULL, 0};

void dbuf_append(struct DynamicBuffer *dbuf, const char *s, int len);
void dbuf_free(struct DynamicBuffer *dbuf);

unsigned int saturating_sub(unsigned int n1, unsigned int n2);


#endif