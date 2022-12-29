#ifndef STRING_H
#define STRING_H

#include <stddef.h>

char* __utoa(unsigned long value, char* str, int base);
char* __itoa(int value, char* str);
char* __strcat(char* dest, char* src);
const char* __strchr(const char* str, int c);
size_t __strcspn(const char* str1, const char* str2);
size_t __strlen(const char* start);
size_t __strspn(const char* str1, const char* str2);
char* __strtok(char* str, const char* del);
void* __memset(void* base, unsigned char val, size_t len);
void* __memcpy(void* dest, const void* src, size_t n);
int __memcmp(const void* str1, const void* str2, size_t n);

#endif // STRING_H