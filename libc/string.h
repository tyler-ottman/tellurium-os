#include <stddef.h>

char * __utoa(unsigned long value, char *str, int base);
char * __itoa(int value, char* str);
size_t __strlen(const char* start);
void* __memset(void* base, unsigned char val, size_t len);
void* __memcpy(void* dest, const void* src, size_t n);
int __memcmp(const void* str1, const void* str2, size_t n);