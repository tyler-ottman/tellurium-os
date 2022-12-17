#include <arch/process.h>
#include <libc/kmalloc.h>
#include <memory/vmm.h>

static struct pcb* kernel_process = NULL;

struct pcb* get_kernel_process() {
    return kernel_process;
}

void init_kernel_process() {
    kernel_process = kmalloc(sizeof(struct pcb));

    kernel_process->pid = KERNEL_PROCESS_ID;
    kernel_process->state = alive;
    VECTOR_ALLOC(kernel_process->threads);
    VECTOR_ALLOC(kernel_process->children);
    kernel_process->pmap = get_kernel_pagemap();
}