#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void *__memcpy(void *dest, const void *src, size_t n);
void *__memset(void *base, unsigned char val, size_t len);
size_t __strlen(const char *start);

#endif // STRING_H