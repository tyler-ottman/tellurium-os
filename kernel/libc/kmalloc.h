#ifndef KMALLOC_H
#define KMALLOC_H

#include <memory/slab.h>
#include <stddef.h>

void *kmalloc(size_t size);
void *krealloc(void *addr, size_t size);
void kfree(void *addr);

#endif // KMALLOC_H