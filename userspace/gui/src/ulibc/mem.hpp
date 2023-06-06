#ifndef MEM_H
#define MEM_H

#include <cstddef>
#include <sys/mman.h>

void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void *addr);
void operator delete[](void *addr);

void *user_malloc(size_t size);
void user_free(void *addr);
void *user_realloc(void *addr, size_t size);

void *__memcpy(void *dest, const void *src, size_t n);

#endif // MEM_H