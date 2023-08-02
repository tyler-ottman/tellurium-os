#ifndef CPU_H
#define CPU_H

#include <devices/msr.h>
#include <klib/vector.h>
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

// core-local information
typedef struct core {
    struct core *self;
    uint64_t *kernel_stack;     // Kernel stack of running thread
    uint64_t kernel_scratch;    // Kernel scratch register for current thread
    uint64_t *irq_stack;
    uint32_t lapic_id;
    uint32_t lapic_ipi_vector;
    uint8_t lapic_timer_vector;
    uint64_t lapic_freq;
    struct tcb* idle_thread;
    struct tcb* current_thread;
    struct TSS tss;
} core_t;

void cpuid(uint32_t in_a, uint32_t a, uint32_t b, uint32_t c, uint32_t d);

void enable_interrupts(void);
void disable_interrupts(void);
int core_get_if_flag(void);
void core_hlt(void);

struct tcb *get_thread_local(void);
struct core *get_core_local_info(void);
void set_core_local_info(struct core* core);

void save_context(ctx_t *ctx);
void print_context(ctx_t* context);
void init_cpu(void);
void core_init(struct limine_smp_info* limine_core_info);

#endif // CPU_H