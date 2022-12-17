#include <arch/lock.h>
#include <arch/thread.h>
#include <libc/vector.h>

static spinlock_t tid_lock = 0;
static uint32_t cur_tid = 0;

uint32_t get_new_tid(void) {
    uint32_t new_tid;
    spinlock_acquire(&tid_lock);
    new_tid = cur_tid++;
    spinlock_release(&tid_lock);
    return new_tid;
}