#ifndef LOCK_H
#define LOCK_H

#include <stdint.h>

typedef uint64_t spinlock_t;

static inline void spinlock_acquire(spinlock_t* spinlock) {
    while(!__sync_bool_compare_and_swap(spinlock, 0, 1)) {
        __asm__ volatile("pause");
    }
}

static inline void spinlock_release(spinlock_t* spinlock) {
    __sync_bool_compare_and_swap(spinlock, 1, 0);
}

#endif // LOCK_H