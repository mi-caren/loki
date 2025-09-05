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

// String strNew(size_t initial_size) {
//     if (initial_size == 0) {
//         initial_size = 1;
//     }

//     size_t cap = 1;
//     while (cap < initial_size) {
//         cap *= 2;
//     }

//     char* buf = malloc(sizeof(StrHeader) + cap + 1);
//     if (buf == NULL)
//         return NULL;

//     StrHeader* header = (StrHeader*)buf;
//     header->len = 0;
//     header->cap = cap;
//     header->cur = 0;

//     String str = buf + sizeof(StrHeader);
//     str[0] = '\0';

//     return str;
// }
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

// String strFromStr(char* str) {
//     size_t str_len = strlen(str);
//     String new_str = strNew(str_len);
//     memcpy(new_str, str, str_len);
//     STR_HEAD(new_str)->len = str_len;
//     new_str[str_len] = '\0';
//     return new_str;
// }
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

// inline void strEmpty(String str) {
//     STR_HEAD(str)->len = 0;
//     str[0] = '\0';
// }
void str_empty(String *self) {
    vec_empty(self->vec);
    _str_push0(self);
}

// inline size_t strLen(String str) {
//     return STR_HEAD(str)->len;
// }
size_t str_len(String *self) {
    return self->vec->len - 1;
}

char* str_chars(String* self) {
    return self->vec->items;
}

// char strBegin(String str) {
//     STR_HEAD(str)->cur = 0;
//     return str[0];
// }

// char strNext(String str) {
//     if (STR_HEAD(str)->cur >= strLen(str) - 1)
//         return '\0';
//     return str[++STR_HEAD(str)->cur];
// }

/* Double the capacity of the string */
// static String _strGrow(String* str) {
//     size_t cap = STR_HEAD(*str)->cap;
//     char* new = (char*)realloc(
//         STR_HEAD(*str),
//         sizeof(StrHeader) + cap*2 + 1
//     );
//     if (new == NULL)
//         return NULL;

//     *str = new + sizeof(StrHeader);
//     STR_HEAD(*str)->cap *= 2;

//     return *str;
// }

// String strInsertChar(String* str, size_t pos, char c) {
//     if (pos > STR_HEAD(*str)->len)
//         return NULL;

//     if (STR_HEAD(*str)->len == STR_HEAD(*str)->cap) {
//         if (_strGrow(str) == NULL)
//             return NULL;
//     }

//     memmove(&((*str)[pos + 1]), &((*str)[pos]), STR_HEAD(*str)->len - pos + 1);
//     memset(&((*str)[pos]), c, 1);
//     STR_HEAD(*str)->len++;

//     return *str;
// }
String* str_insertc(String* self, char c, size_t pos) {
    if (pos > str_len(self)) return NULL;
    if (!vec_insert(self->vec, c, pos)) return NULL;
    return self;
}

// static String _strMakeSpace(String* str, size_t space) {
//     size_t cap = STR_HEAD(*str)->cap;
//     if (cap >= space)
//         return *str;

//     while (cap < space) {
//         cap *= 2;
//     }

//     char* new = (char*)realloc(
//         STR_HEAD(*str),
//         sizeof(StrHeader) + cap + 1
//     );
//     if (new == NULL)
//         return NULL;

//     *str = new + sizeof(StrHeader);
//     STR_HEAD(*str)->cap = cap;

//     return *str;
// }

// String strInsert(String* str, size_t pos, char* str2) {
//     if (pos > STR_HEAD(*str)->len)
//         return NULL;

//     size_t new_space = strlen(str2);
//     size_t total_space = STR_HEAD(*str)->len + new_space;
//     if (total_space > STR_HEAD(*str)->cap) {
//         if (_strMakeSpace(str, total_space) == NULL)
//             return NULL;
//     }

//     memmove(&((*str)[pos + new_space]), &((*str)[pos]), STR_HEAD(*str)->len - pos + 1);
//     memcpy(&((*str)[pos]), str2, new_space);
//     STR_HEAD(*str)->len += new_space;

//     return *str;
// }
String* str_insert(String* self, size_t pos, char* chars) {
    if (pos > str_len(self)) return NULL;
    // TODO: vec_insert_array
    for (size_t i = 0; i < strlen(chars); i++)
        assert(vec_insert(self->vec, chars[i], pos+i));
    return self;
}

// String strAppend(String* str, char* str2) {
//     size_t str_len = STR_HEAD(*str)->len;
//     size_t str2_len = strlen(str2);
//     size_t total_space = str_len + str2_len;
//     if (total_space > STR_HEAD(*str)->cap) {
//         if (_strMakeSpace(str, total_space) == NULL)
//             return NULL;
//     }

//     memcpy(&((*str)[str_len]), str2, str2_len);
//     STR_HEAD(*str)->len += str2_len;
//     (*str)[STR_HEAD(*str)->len] = '\0';

//     return *str;
// }
String* str_append(String* self, char* chars) {
    // TODO: vec_append
    vec_pop(self->vec);
    for (size_t i = 0; i < strlen(chars); i++)
        assert(vec_push(self->vec, chars[i]));
    _str_push0(self);
    return self;
}

// String strAppendChar(String* str, char c) {
//     if (STR_HEAD(*str)->len == STR_HEAD(*str)->cap) {
//         if (_strGrow(str) == NULL)
//             return NULL;
//     }

//     (*str)[STR_HEAD(*str)->len++] = c;
//     (*str)[STR_HEAD(*str)->len] = '\0';

//     return *str;
// }
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

// String strRepeatAppendChar(String* str, char c, size_t n) {
//     size_t str_len = STR_HEAD(*str)->len;
//     size_t total_space = str_len + n;
//     if (total_space > STR_HEAD(*str)->cap) {
//         if (_strMakeSpace(str, total_space) == NULL)
//             return NULL;
//     }

//     memset(&((*str)[str_len]), c, n);
//     STR_HEAD(*str)->len += n;
//     (*str)[STR_HEAD(*str)->len] = '\0';

//     return *str;
// }
String* str_repeat_appendc(String* self, char c, size_t n) {
    vec_pop(self->vec);
    if (!vec_repeat_append(self->vec, c, n))
        return NULL;
    _str_push0(self);
    return self;
}

// char strRemove(String str, size_t pos) {
//     if (pos >= STR_HEAD(str)->len)
//         return '\0';

//     char c = str[pos];
//     memmove(&str[pos], &str[pos + 1], STR_HEAD(str)->len - pos);
//     STR_HEAD(str)->len--;

//     return c;
// }
char str_remove(String* self, size_t pos) {
    if (pos >= str_len(self)) return '\0';
    char c = self->vec->items[pos];
    vec_remove(self->vec, pos);
    return c;
}

// String strTruncate(String str, size_t new_len) {
//     if (new_len > STR_HEAD(str)->len)
//         return str;
//     str[new_len] = '\0';
//     STR_HEAD(str)->len = new_len;
//     return str;
// }
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
    assert(pos < str_len(self));
    return self->vec->items[pos];
}

// char strSetAt(String str, size_t pos) {
//     if (pos >= STR_HEAD(str)->len)
//         return '\0';
//     STR_HEAD(str)->cur = pos;
//     return str[pos];
// }
void str_set_at(String* self, size_t pos) {
    assert(pos < str_len(self));
    self->vec->curr = pos;
}

// inline size_t strCurrIdx(String str) {
//     return STR_HEAD(str)->cur;
// }
size_t str_curri(String* self) {
    return self->vec->curr;
}

// void strFree(String str) {
//     free(STR_HEAD(str));
// }
void str_free(String* self) {
    vec_free(self->vec);
}

