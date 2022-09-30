#include <stddef.h>

char * __utoa(unsigned long value, char *str, int base);
char * __itoa(int value, char* str);
size_t __strlen(const char* start);
void* __memset(void* base, int val, int len);