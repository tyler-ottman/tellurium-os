#ifndef CPU_H
#define CPU_H

#include <memory/pmm.h>
#include <libc/vector.h>
#include <limine.h>

#define KERNEL_THREAD_STACK_SIZE    (2 * PAGE_SIZE_BYTES)

struct context {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t err;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

void enable_interrupts(void);
void disable_interrupts(void);
void init_cpu(void);
void core_init(struct limine_smp_info* core);

#endif // CPU_H