#ifndef STRING_H
#define STRING_H

#include <unistd.h>

typedef char* String;


#define STR_NEW()                   strNew(0)
#define STR_NEW_WITH_CAP(CAP)       strNew(CAP)

#define STR_FOREACH(C, STR) \
    for (char C = strBegin(STR); C != '\0'; C = strNext(STR))

#define STR_FOR(IDX, STR) \
    for (size_t IDX = 0; IDX < strLen(STR); IDX++)


String strNew(size_t initial_size);
String strFromStr(char* str);

void strEmpty(String str);
size_t strLen(String str);
char strBegin(String str);
char strNext(String str);
String strInsertChar(String* str, size_t pos, char c);
String strInsert(String* str, size_t pos, char* str2);
char strRemove(String str, size_t pos);
void strFree(String str);
/* str2 MUST be 0 terminated */
String strAppend(String* str, char* str2);
String strAppendChar(String* str, char c);
String strRepeatAppendChar(String* str, char c, size_t n);
String strTruncate(String str, size_t new_len);

char strSetAt(String str, size_t pos);
size_t strCurrIdx(String str);

#endif