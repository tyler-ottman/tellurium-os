#ifndef MEM_H
#define MEM_H

#include <stddef.h>

// Temp location
#define MAP_FAILED                      ((void *) -1)
#define MAP_SHARED                      0x1
#define MAP_ANONYMOUS                   0x20

#define PROT_READ                       0x1
#define PROT_WRITE                      0x2


void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void *addr);
void operator delete[](void *addr);
void operator delete(void*, unsigned long);

void *user_malloc(size_t size);
void user_free(void *addr);
void *user_realloc(void *addr, size_t size);

#endif // MEM_H