#ifndef SLAB_H
#define SLAB_H

#include <stdint.h>
#include <arch/lock.h>
#include <memory/pmm.h>

#define SLAB_CHUNK_SIZE_VARIENTS    16

#define SLAB_MIN_CHUNK_SIZE         8
#define SLAB_MAX_CHUNK_SIZE         1024

struct slab {
    struct cache *cache;
    struct slab *prev;
    struct slab *next;
    uint16_t used_chunks;
    uint16_t total_chunks;
    void *free_chunk;
}__attribute__((packed));

struct cache {
    struct slab *slabs_empty;
    struct slab *slabs_partial;
    struct slab *slabs_full;
    spinlock_t lock;
    size_t chunk_size;
};

struct cache_metadata {
    size_t cur_caches;
    size_t max_caches;
};

struct page_metadata {
    size_t pages_allocated;
};

void cache_insert(size_t chunk_size);
int get_cache_index(size_t chunk_size);

void transfer_slab(struct slab *slab, struct slab **old_list, struct slab **new_list);
void slab_spawn(struct cache *cache);

void *slab_inner_alloc_chunk(struct slab *slab_list_base);
void *slab_alloc_chunk(int cache_idx);
void slab_free_chunk(void *addr);

void *slab_alloc(size_t size);
void *slab_realloc(void *addr, size_t size);
void slab_free(void *addr);

void init_slab(void);

#endif // SLAB_H