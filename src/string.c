#include "utils/string.h"
#include <stdlib.h>
#include <string.h>


static String _strGrow(String* str);

#define STR_HEAD(STR)    ((StrHeader*)(STR - sizeof(StrHeader)))

typedef struct {
    size_t len;
    size_t cap;
    size_t cur;
} StrHeader;

String strNew(size_t initial_size) {
    if (initial_size == 0) {
        initial_size = 1;
    }

    size_t cap = 1;
    while (cap < initial_size) {
        cap *= 2;
    }

    char* buf = malloc(sizeof(StrHeader) + cap + 1);
    if (buf == NULL)
        return NULL;

    StrHeader* header = (StrHeader*)buf;
    header->len = 0;
    header->cap = cap;
    header->cur = 0;

    String str = buf + sizeof(StrHeader);
    str[0] = '\0';

    return str;
}

String strFromStr(char* str) {
    size_t str_len = strlen(str);
    String new_str = strNew(str_len);
    memcpy(new_str, str, str_len);
    STR_HEAD(new_str)->len = str_len;
    new_str[str_len] = '\0';
    return new_str;
}

inline void strEmpty(String str) {
    STR_HEAD(str)->len = 0;
    str[0] = '\0';
}

inline size_t strLen(String str) {
    return STR_HEAD(str)->len;
}

char strBegin(String str) {
    STR_HEAD(str)->cur = 0;
    return str[0];
}

char strNext(String str) {
    if (STR_HEAD(str)->cur >= strLen(str) - 1)
        return '\0';
    return str[++STR_HEAD(str)->cur];
}

/* Double the capacity of the string */
static String _strGrow(String* str) {
    size_t cap = STR_HEAD(*str)->cap;
    char* new = (char*)realloc(
        STR_HEAD(*str),
        sizeof(StrHeader) + cap*2 + 1
    );
    if (new == NULL)
        return NULL;

    *str = new + sizeof(StrHeader);
    STR_HEAD(*str)->cap *= 2;

    return *str;
}

String strInsertChar(String* str, size_t pos, char c) {
    if (pos > STR_HEAD(*str)->len)
        return NULL;

    if (STR_HEAD(*str)->len == STR_HEAD(*str)->cap) {
        if (_strGrow(str) == NULL)
            return NULL;
    }

    memmove(&((*str)[pos + 1]), &((*str)[pos]), STR_HEAD(*str)->len - pos + 1);
    memset(&((*str)[pos]), c, 1);
    STR_HEAD(*str)->len++;

    return *str;
}

static String _strMakeSpace(String* str, size_t space) {
    size_t cap = STR_HEAD(*str)->cap;
    if (cap >= space)
        return *str;

    while (cap < space) {
        cap *= 2;
    }

    char* new = (char*)realloc(
        STR_HEAD(*str),
        sizeof(StrHeader) + cap + 1
    );
    if (new == NULL)
        return NULL;

    *str = new + sizeof(StrHeader);
    STR_HEAD(*str)->cap = cap;

    return *str;
}

String strInsert(String* str, size_t pos, char* str2) {
    if (pos > STR_HEAD(*str)->len)
        return NULL;

    size_t new_space = strlen(str2);
    size_t total_space = STR_HEAD(*str)->len + new_space;
    if (total_space > STR_HEAD(*str)->cap) {
        if (_strMakeSpace(str, total_space) == NULL)
            return NULL;
    }

    memmove(&((*str)[pos + new_space]), &((*str)[pos]), STR_HEAD(*str)->len - pos + 1);
    memcpy(&((*str)[pos]), str2, new_space);
    STR_HEAD(*str)->len += new_space;

    return *str;
}

String strAppend(String* str, char* str2) {
    size_t str_len = STR_HEAD(*str)->len;
    size_t str2_len = strlen(str2);
    size_t total_space = str_len + str2_len;
    if (total_space > STR_HEAD(*str)->cap) {
        if (_strMakeSpace(str, total_space) == NULL)
            return NULL;
    }

    memcpy(&((*str)[str_len]), str2, str2_len);
    STR_HEAD(*str)->len += str2_len;
    (*str)[STR_HEAD(*str)->len] = '\0';

    return *str;
}

String strAppendChar(String* str, char c) {
    if (STR_HEAD(*str)->len == STR_HEAD(*str)->cap) {
        if (_strGrow(str) == NULL)
            return NULL;
    }

    (*str)[STR_HEAD(*str)->len++] = c;
    (*str)[STR_HEAD(*str)->len] = '\0';

    return *str;
}

String strRepeatAppendChar(String* str, char c, size_t n) {
    size_t str_len = STR_HEAD(*str)->len;
    size_t total_space = str_len + n;
    if (total_space > STR_HEAD(*str)->cap) {
        if (_strMakeSpace(str, total_space) == NULL)
            return NULL;
    }

    memset(&((*str)[str_len]), c, n);
    STR_HEAD(*str)->len += n;
    (*str)[STR_HEAD(*str)->len] = '\0';

    return *str;
}

char strRemove(String str, size_t pos) {
    if (pos >= STR_HEAD(str)->len)
        return '\0';

    char c = str[pos];
    memmove(&str[pos], &str[pos + 1], STR_HEAD(str)->len - pos);
    STR_HEAD(str)->len--;

    return c;
}

String strTruncate(String str, size_t new_len) {
    if (new_len > STR_HEAD(str)->len)
        return str;
    str[new_len] = '\0';
    STR_HEAD(str)->len = new_len;
    return str;
}

char strSetAt(String str, size_t pos) {
    if (pos >= STR_HEAD(str)->len)
        return '\0';
    STR_HEAD(str)->cur = pos;
    return str[pos];
}

inline size_t strCurrIdx(String str) {
    return STR_HEAD(str)->cur;
}

void strFree(String str) {
    free(STR_HEAD(str));
}
