#ifndef _STRING_H_
#define _STRING_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma section code_type ".init"
void* memcpy(void* dst, const void* src, size_t n);
void* memset(void* dst, int val, size_t n);
void __fill_mem(void* dst, int val, size_t n);
#pragma section code_type

size_t strlen(const char* s);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t num);
int strcmp(const char* s1, const char* s2);
int _stricmp(const char* s1, const char* s2);
char* _strlwr(char* string);
int strncmp(const char* s1, const char* s2, size_t n);
char* strncat(char* dest, const char* src, size_t n);
char* strcat(char* dest, const char* src);
char* strstr(const char* str, const char* pat);
char* strtok(char* str, const char* set);
char* strchr(const char* str, int chr);
char* strrchr(const char* str, int chr);

int memcmp(const void* a, const void* b, size_t n);
void* memmove(void* dst, const void* src, size_t n);

#ifdef __cplusplus
}
#endif

#endif
