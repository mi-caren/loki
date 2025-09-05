#ifndef STRING_H
#define STRING_H

#include <unistd.h>

#include "aeolus/iterator.h"
#include "vec.h"

// struct Iterator(String);

typedef struct {
    Vec(char)* vec;
    ITER_DRIVER(String);
} String;

ITER_DEFS(String, char)

String str_new();
String str_new_with_cap(size_t initial_size);
String str_from(char* chars);

void str_empty(String* self);
size_t str_len(String* self);
// char strBegin(String str);
// char strNext(String str);
String* str_insertc(String* self, char c, size_t pos);
String* str_insert(String* self, size_t pos, char* chars);
// String* str_insert_str(String* self, String* str);
char str_remove(String* self, size_t pos);
/* str2 MUST be 0 terminated */
String* str_append(String* self, char* chars);
String* str_appendc(String* self, char c);
String* str_repeat_appendc(String* self, char c, size_t n);
void str_trucate(String* self, size_t new_len);
void str_free(String* self);

// TODO: iterator methods
void str_set_at(String* self, size_t pos);
size_t str_curri(String* self);

#endif