#include "string.h"

void* __memcpy(void* dest, const void* src, size_t n) {
    char* dest_ptr = (char*)dest;
    const char* src_ptr = (const char*)src;
    while (n-- > 0) {
        *(dest_ptr++) = *(src_ptr++);
    }
    return dest;
}

size_t __strlen(const char *str) {
    const char *start = str;
    while(*str != '\0')
        str++;
    return (size_t)(str - start);
}