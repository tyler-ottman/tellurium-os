#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

char* __utoa(uint64_t value, char* str, int base);
char* __itoa(int64_t value, char* str);
char* __strcat(char* dest, char* src);
const char* __strchr(const char* str, int c);
size_t __strcspn(const char* str1, const char* str2);
size_t __strlen(const char* start);
size_t __strncmp(const char *str1, const char *str2, size_t n);
const char *__strncpy(char *dest, const char *src, size_t n);
size_t __strspn(const char* str1, const char* str2);
char* __strtok(char* str, const char* del);
char *__strtok_r(char *str, const char *delim, char **buff);
void* __memset(void* base, unsigned char val, size_t len);
void* __memcpy(void* dest, const void* src, size_t n);
int __memcmp(const void* str1, const void* str2, size_t n);

#endif // STRING_H