#include "string.h"

void* __memcpy(void* dest, const void* src, size_t n) {
    char* dest_ptr = (char*)dest;
    const char* src_ptr = (const char*)src;
    while (n-- > 0) {
        *(dest_ptr++) = *(src_ptr++);
    }
    return dest;
}

void *__memset(void *base, unsigned char val, size_t len) {
    char *ptr = (char *)base;
    
    while (len-- > 0) {
        *(ptr++) = val;
    }

    return base;
}

size_t __strlen(const char *str) {
    const char *start = str;
    while(*str != '\0')
        str++;
    return (size_t)(str - start);
}