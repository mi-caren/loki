#include <bits/types/struct_iovec.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "aeolus/string.h"
#include "aeolus/iterator.h"
#include "aeolus/vec.h"


// static String _strGrow(String* str);

ITER_IMPL(
    String,
    /* CURR_IMPL */ {
        size_t len = str_len(self);
        if (len == 0) return NULL;
        if (self->vec->curr >= len) return NULL;
        return &self->vec->items[self->vec->curr];
    },
    /* BEGIN_IMPL */ {
        self->vec->curr = 0;
        return iter_curr(self);
    },
    /* END_IMPL */ {
        size_t len = str_len(self);
        if (len == 0) return NULL;
        self->vec->curr = len - 1;
        return iter_curr(self);
    },
    /* PREV_IMPL */ {
        return iter_prev(self->vec);
    },
    /* NEXT_IMPL */ {
        size_t len = str_len(self);
        if (self->vec->curr  >= len - 1) return NULL;
        self->vec->curr++;
        return iter_curr(self);
    }
)

static void _str_push0(String* self) {
    assert(vec_push(self->vec, '\0'));
}

String str_new_with_cap(size_t initial_size) {
    if (initial_size < 2) initial_size = 2;
    Vec(char)* vec = vec_new_with_cap(char, initial_size);
    assert(vec != NULL);
    assert(vec_push(vec, '\0'));
    String str;
    str.vec = vec;
    ITER_INIT(String, &str);
    return str;
}

String str_new() {
    return str_new_with_cap(2);
}

String str_from(char *chars) {
    size_t len = strlen(chars);
    String str = str_new_with_cap(len + 1);
    assert(vec_pop(str.vec));
    // TODO: create vec_from
    for (size_t i = 0; i < len; i ++)
        assert(vec_push(str.vec, chars[i]));
    _str_push0(&str);
    return str;
}

void str_empty(String *self) {
    vec_empty(self->vec);
    _str_push0(self);
}

size_t str_len(String *self) {
    return self->vec->len - 1;
}

char* str_chars(String* self) {
    return self->vec->items;
}

String* str_insertc(String* self, char c, size_t pos) {
    if (pos > str_len(self)) return NULL;
    if (!vec_insert(self->vec, c, pos)) return NULL;
    return self;
}

String* str_insert(String* self, char* chars, size_t pos) {
    if (pos > str_len(self)) return NULL;
    // TODO: vec_insert_array
    for (size_t i = 0; i < strlen(chars); i++)
        assert(vec_insert(self->vec, chars[i], pos+i));
    return self;
}

String* str_append(String* self, char* chars) {
    // TODO: vec_append
    vec_pop(self->vec);
    for (size_t i = 0; i < strlen(chars); i++)
        assert(vec_push(self->vec, chars[i]));
    _str_push0(self);
    return self;
}

String* str_appendc(String* self, char c) {
    vec_pop(self->vec);
    assert(vec_push(self->vec, c));
    _str_push0(self);
    return self;
}

String* str_appends(String*self, String* str) {
    vec_pop(self->vec);
    for (size_t i = 0; i < str_len(str); i++)
        assert(vec_push(self->vec, str->vec->items[i]));
    _str_push0(self);
    return self;
}

String* str_repeat_appendc(String* self, char c, size_t n) {
    vec_pop(self->vec);
    if (!vec_repeat_append(self->vec, c, n))
        return NULL;
    _str_push0(self);
    return self;
}

char str_remove(String* self, size_t pos) {
    if (pos >= str_len(self)) return '\0';
    char c = self->vec->items[pos];
    vec_remove(self->vec, pos);
    return c;
}

void str_truncate(String* self, size_t new_len) {
    if (new_len > str_len(self))
        return;
    vec_pop(self->vec);
    while (self->vec->len > new_len) {
        vec_pop(self->vec);
    }
    _str_push0(self);
}

char str_char_at(String* self, size_t pos) {
    assert(pos < self->vec->len);
    return self->vec->items[pos];
}

void str_set_at(String* self, size_t pos) {
    assert(pos < str_len(self));
    self->vec->curr = pos;
}

size_t str_curri(String* self) {
    return self->vec->curr;
}

void str_free(String* self) {
    vec_free(self->vec);
}

