#ifndef STRING_H
#define STRING_H

#include <bits/types/struct_iovec.h>
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
char* str_chars(String* self);
String* str_insertc(String* self, char c, size_t pos);
String* str_insert(String* self, char* chars, size_t pos);
char str_remove(String* self, size_t pos);
/* chars MUST be 0 terminated */
String* str_append(String* self, char* chars);
String* str_appendc(String* self, char c);
String* str_appends(String*self, String* str);
String* str_repeat_appendc(String* self, char c, size_t n);
void str_truncate(String* self, size_t new_len);
char str_char_at(String* self, size_t pos);
void str_free(String* self);

// TODO: iterator methods
void str_set_at(String* self, size_t pos);
size_t str_curri(String* self);

#endif