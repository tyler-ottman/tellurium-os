#ifndef CPU_H
#define CPU_H

// #include <arch/gdt.h>
#include <libc/vector.h>
#include <limine.h>

#define RFLAGS_RESERVED_MASK        0x0002
#define RFLAGS_INTERRUPT_MASK       0x0200

#define KERNEL_THREAD_STACK_SIZE    (2 * PAGE_SIZE_BYTES)

struct TSS {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t io_map_base_addr;
}__attribute__((packed));

typedef struct ctx {
    uint64_t ds;
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
} ctx_t;

struct core_local_info {
    uint64_t* abort_stack;
    uint32_t lapic_id;
    uint8_t lapic_timer_vector;
    struct tcb* idle_thread;
    struct tcb* current_thread;
    struct TSS tss;
};

void done(void);
struct tcb* get_thread_local(void);
void set_thread_local(struct tcb* thread);
struct core_local_info* get_core_local_info(void);
void set_core_local_info(struct core_local_info* cpu_info);
void enable_interrupts(void);
void disable_interrupts(void);
void print_context(ctx_t* context);
void init_cpu(void);
void core_init(struct limine_smp_info* core);

#endif // CPU_H