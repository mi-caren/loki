#ifndef UTILS_H
#define UTILS_H


struct DynamicBuffer {
    char *b;
    int len;
};

#define ABUF_INIT {NULL, 0};


void ab_append(struct DynamicBuffer *ab, const char *s, int len);
void ab_free(struct DynamicBuffer *ab);


#endif
