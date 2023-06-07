#include "mem.hpp"
#include "syscalls.hpp"
#include "ulibc/string.h"

#define MAX(X,Y) ((X) < (Y) ? (Y) : (X))

#define PAGE_SIZE_BYTES             4096
#define MAGIC                       0x77A5844CU

typedef struct  {
    size_t block_size;              // Includes size of header
    void *next;
} FreeBlock;

typedef struct {
    size_t magic;
    size_t len;                     // Includes size of header
} MallocHeader;

static FreeBlock malloc_head = {
    .next = nullptr
};

void *operator new(size_t size) {
    return user_malloc(size);
}

void *operator new[](size_t size) {
    return user_malloc(size);
}

void operator delete(void *addr) {
    user_free(addr);
}

void operator delete[](void *addr) {
    user_free(addr);
}

// Align num up to next number divisible by bound
static inline size_t alignUp(size_t num, size_t bound) {
    size_t ret = num / bound;
    if (num % bound != 0) {
        ret++;
    }
    return ret * bound;
}

static void *requestBlock(size_t size) {
    void *addr = syscall_mmap(nullptr, size, PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) {
        return nullptr;
    }
    return addr;
}

void *user_malloc(size_t size) {
    if (size == 0) {
        return nullptr;
    }

    size = alignUp(size + MAX(sizeof(MallocHeader), sizeof(FreeBlock)), sizeof(size_t));
    FreeBlock *cur_block = (FreeBlock *)(&malloc_head)->next;
    FreeBlock *prev_block = &malloc_head;

    for (;;) {
        // Call mmap if end of free list reached
        if (cur_block == nullptr) {
            // Block allocations rounded up to nearest page
            size_t aligned_size = alignUp(size, PAGE_SIZE_BYTES);
            MallocHeader *addr = (MallocHeader *)requestBlock(aligned_size);
            addr->magic = MAGIC;
            addr->len = size;

            size_t rem_size = aligned_size - size;
            if (rem_size > sizeof(FreeBlock)) { // Add free block
                size_t free_addr = (size_t)addr + size;
                FreeBlock *free_blk = (FreeBlock *)free_addr;
                free_blk->block_size = rem_size;
                free_blk->next = nullptr;
                prev_block->next = (FreeBlock *)free_blk;
            } else { // Append remaining size to current allocated block
                addr->len += rem_size;
            }

            return (void *)((size_t)addr + sizeof(MallocHeader));
        }

        // Check if current free chunk can fit requested size
        if (size <= cur_block->block_size) {
            size_t rem_size = cur_block->block_size - size;
            if (rem_size > sizeof(FreeBlock)) { // Add free block
                size_t free_addr = (size_t)cur_block + size;
                FreeBlock *free_blk = (FreeBlock *)free_addr;
                free_blk->block_size = rem_size;
                free_blk->next = cur_block->next;
                prev_block->next = free_blk;
            } else { // Append remaining free block to allocated block
                size += rem_size;
                prev_block->next = cur_block->next;
            }

            MallocHeader *blk = (MallocHeader *)cur_block;
            blk->magic = MAGIC;
            blk->len = size;
            return (void *)((size_t)blk + sizeof(MallocHeader));
        }

        prev_block = cur_block;
        cur_block = (FreeBlock *)cur_block->next;
    }

    return nullptr;
}

void user_free(void *addr) {
    MallocHeader *header = (MallocHeader *)((size_t)addr - sizeof(MallocHeader));
    if (addr == nullptr || header->magic != MAGIC) {
        return;
    }
    
    FreeBlock *cur_block = (FreeBlock *)(&malloc_head)->next;
    FreeBlock *prev_block = &malloc_head;

    for (;;) {
        if (cur_block == nullptr) {
            size_t size = header->len;
            FreeBlock *free_blk = (FreeBlock *)header;
            free_blk->block_size = size;
            free_blk->next = nullptr;
            prev_block->next = free_blk;

            return;
        }

        prev_block = cur_block;
        cur_block = (FreeBlock *)cur_block->next;
    }
}

void *user_realloc(void *addr, size_t size) {
    MallocHeader *header = (MallocHeader *)((size_t)addr - sizeof(MallocHeader));
    if (addr == nullptr || header->magic != MAGIC) {
        return nullptr;
    }

    size_t old_size = header->len - sizeof(MallocHeader);
    if (old_size >= size) {
        return addr;
    }

    void *new_addr = user_malloc(size);
    if (new_addr == nullptr) {
        return nullptr;
    }

    __memcpy(new_addr, addr, old_size);
    return new_addr;
}