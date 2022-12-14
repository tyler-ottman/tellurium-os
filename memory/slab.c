#include <arch/terminal.h>
#include <memory/pmm.h>
#include <memory/slab.h>

static struct cache* caches[SLAB_CHUNK_SIZE_VARIENTS];
static struct cache_metadata metadata;

static size_t known_chunks[] = {
    8, 16, 32, 64, 128, 256, 512, 1024
};

void cache_insert(size_t chunk_size) {
    if (metadata.cur_caches == metadata.max_caches) {
        kerror("slab: max caches reached\n");
    }

    if (chunk_size > SLAB_MAX_CHUNK_SIZE) {
        kerror("slab: cache object size exceeded\n");
    }
    
    struct cache* cache = palloc(1);
    cache->slabs_empty = cache->slabs_partial = cache->slabs_full = NULL;
    cache->chunk_size = chunk_size;
    
    caches[metadata.cur_caches++] = cache;    
}

int get_cache_index(size_t chunk_size) {
    size_t chunk_contender;
    for (size_t i = 0; i < metadata.max_caches; i++) {
        chunk_contender = caches[i]->chunk_size;
        if (chunk_size <= chunk_contender) {
            return i;
        }
    }
    return -1;
}

void transfer_slab(struct slab* slab) {
    bool is_full = slab->used_chunks == slab->total_chunks;
    bool is_empty = slab->used_chunks == 0;
    
    struct cache* cache = slab->cache;
    struct slab** old_list;
    struct slab** new_list;
    
    if (is_full) { // partial -> full
        old_list = &(cache->slabs_partial);
        new_list = &(cache->slabs_full);
    } else if (is_empty) { // partial -> empty
        old_list = &(cache->slabs_partial);
        new_list = &(cache->slabs_empty);
    } else {
        if (slab->used_chunks == 1) { // empty -> partial
            old_list = &(cache->slabs_empty);
            new_list = &(cache->slabs_partial);
        } else { // full -> partial
            old_list = &(cache->slabs_full);
            new_list = &(cache->slabs_partial);
        }
    }

    if (slab->prev == NULL && slab->next == NULL) { // Only slab in list
        
        *old_list = NULL;
    } else if (slab->next == NULL) { // Slab at end of list
        slab->prev->next = NULL;
    } else if (slab->prev == NULL) { // Slab at beginning of list
        slab->next->prev = NULL;
        *old_list = slab->next;
    } else { // Slab in middle of list
        slab->prev->next = slab->next;
        slab->next->prev = slab->prev;
    }
    
    slab->next = *new_list;
    *new_list = slab;    
    slab->prev = NULL;
}

void slab_spawn(struct cache* cache) {
    int chunk_size = cache->chunk_size;
    int total_chunks = PAGE_SIZE_BYTES / chunk_size;
    int header_size = sizeof(struct slab);
    
    struct slab* slab = palloc(1);
    __memset(slab, 0, header_size);

    int header_size_chunks = header_size / chunk_size;
    if (header_size % chunk_size != 0) header_size_chunks++;

    slab->total_chunks = total_chunks - header_size_chunks;
    slab->cache = cache;

    // Insert to partial list
    if (cache->slabs_partial != NULL) {
        slab->next = cache->slabs_partial;
        slab->next->prev = slab;
    }
    cache->slabs_partial = slab;

    // Initialize chunk freelist
    uint64_t base_addr = (uint64_t)cache->slabs_partial;
    base_addr += chunk_size * header_size_chunks;
    slab->free_chunk = (uint64_t*)base_addr;
    void** cur_chunk = (void**)slab->free_chunk;
    size_t offset = chunk_size / sizeof(void*);
    for (size_t i = 0; i < slab->total_chunks; i++) {
        cur_chunk[i * offset] = &cur_chunk[(i + 1) * offset];
    }
    cur_chunk[slab->total_chunks * offset] = NULL;
}

void* slab_inner_alloc_chunk(struct slab* slab_list_base) {
    struct slab* cur_slab = slab_list_base;

    while (cur_slab != NULL) {
        if ((cur_slab->used_chunks != cur_slab->total_chunks)) {
            cur_slab->used_chunks++;
            void** old_free_chunk = cur_slab->free_chunk;
            cur_slab->free_chunk = *old_free_chunk;
            
            // Add slab to full list
            if (cur_slab->used_chunks == cur_slab->total_chunks) {
                transfer_slab(cur_slab);
            }
            return old_free_chunk;
        }
        cur_slab = cur_slab->next;
    }

    return NULL;
}

void* slab_alloc_chunk(int cache_idx) {
    struct cache* cache = caches[cache_idx];
    void* addr;

    // Try getting chunk from partial slab list
    addr = slab_inner_alloc_chunk(cache->slabs_partial);
    if (addr != NULL) {
        return addr;
    }

    // Try getting chunk from empty slab list
    addr = slab_inner_alloc_chunk(cache->slabs_empty);
    if (addr != NULL) {
        return addr;
    }

    // Empty/Partial lists have no chunks available
    slab_spawn(cache);

    return slab_inner_alloc_chunk(cache->slabs_partial);

}

void* slab_alloc(size_t size) {
    int cache_idx = get_cache_index(size);
    if (cache_idx != -1) {
        return slab_alloc_chunk(cache_idx);
    }

    // Bypass slab allocator, jump directly to PMM
    size_t pages = size / PAGE_SIZE_BYTES;
    if (size % PAGE_SIZE_BYTES != 0) pages++;

    return palloc(pages);
}

void slab_free_chunk(void* addr) {
    struct slab* slab = (struct slab*)((uint64_t)addr & ~0xfff);
    
    void **new_free = addr;
    
    *new_free = slab->free_chunk;
    
    slab->free_chunk = new_free;
    
    // Check if slab needs to be relocated
    uint16_t prev_chunks = slab->used_chunks;
    slab->used_chunks--;
    
    if ((slab->used_chunks == 0) || (prev_chunks == slab->total_chunks)) {
        transfer_slab(slab);
    }
}

void slab_free(void* addr) {
    if (addr == NULL) {
        return;
    }

    // Bypass slab de-allocator
    if ((((uint64_t)addr) & 0xfff) == 0) {
        // Todo, free non-slab allocations
        return;
    }

    slab_free_chunk(addr);
}

void* slab_realloc(void* addr, size_t size) {
    void* chunk = NULL;
    if (addr == NULL) {
        return NULL;
    }

    // Bypass slab re-allocator
    if ((((uint64_t)addr) & 0xfff) == 0) {
        // Todo, implement non-slab re-allocations
        return NULL;
    }

    struct slab* slab = (struct slab*)((uint64_t)addr & ~0xfff);
    if (size > slab->cache->chunk_size) {
        chunk = slab_alloc(size);
        if (chunk == NULL) {
            return NULL;
        }

        __memcpy(chunk, slab, slab->cache->chunk_size);
        slab_free_chunk(slab);
    }

    return chunk;
}

void init_slab() {
    metadata.max_caches = sizeof(known_chunks) / sizeof(known_chunks[0]);
    metadata.cur_caches = 0;

    for (size_t i = 0; i < metadata.max_caches; i++) {
        cache_insert(known_chunks[i]);
    }
}