#include <apps/elf.h>
#include <arch/process.h>
#include <arch/scheduler.h>
#include <libc/kmalloc.h>
#include <memory/vmm.h>

static struct pcb* kernel_process = NULL;
static uint16_t pids[PROCESS_MAX] = {0};

struct pcb* get_kernel_process() {
    return kernel_process;
}

void process_destroy(pcb_t *proc) {
    if (!proc) {
        return;
    }

    process_free_pid(proc->pid);
    // TODO, free vector data structures
    
    if (proc->pmap) {
        if (proc->pmap->pml4_base) {
            kfree(proc->pmap->pml4_base);
        }
        kfree(proc->pmap);
    }

    kfree(proc);
}

pcb_t *create_user_process(const char *elf_path) {
    pcb_t *proc = (pcb_t *)kmalloc(sizeof(pcb_t));
    if (!proc) {
        return NULL;
    }

    proc->pid = process_alloc_pid();
    proc->state = ALIVE;
    VECTOR_ALLOC(proc->threads);
    VECTOR_ALLOC(proc->children);

    proc->pmap = kmalloc(sizeof(struct pagemap));
    if (!proc->pmap) {
        process_destroy(proc);
        return NULL;
    }

    proc->pmap->pml4_base = kmalloc(PAGE_SIZE_BYTES);
    if (!proc->pmap->pml4_base) {
        process_destroy(proc);
        return NULL;
    }

    struct pagemap *k_pmap = get_kernel_pagemap();
    __memcpy(proc->pmap->pml4_base, k_pmap->pml4_base, PAGE_SIZE_BYTES);

    // Initialize file descriptor table
    VECTOR_ALLOC(proc->fd_table.fd_table);
    vector_t fd_table_vec = proc->fd_table.fd_table;
    for (int i = 0; i < VECTOR_SIZE(fd_table_vec); i++) {
        VECTOR_SET(fd_table_vec, i, NULL);
    }

    proc->fd_table.fd_table_lock = 0;
    proc->cwd = vfs_get_root();

    // Load ELF into user process's address space
    uint64_t entry;
    elf_load(proc, elf_path, &entry);

    // Create 'main' user thread
    thread_t *thread = create_user_thread(proc, (void *)entry, NULL);
    schedule_add_thread(thread);

    return proc;
}

int process_alloc_pid(void) {
    for (int i = 0; i < PROCESS_MAX; i++) {
        if (!pids[i]) {
            pids[i] = 1;
            return i;
        }
    }
    return -1;
}

void process_free_pid(uint16_t pid) {
    if (pid <= PROCESS_MAX) {
        pids[pid] = 0;
    }
}

void init_kernel_process() {
    kernel_process = kmalloc(sizeof(struct pcb));
    if (!kernel_process) {
        kerror(INFO "Failed to allocate kernel process\n");
    }

    kernel_process->pid = process_alloc_pid();
    kernel_process->state = ALIVE;
    VECTOR_ALLOC(kernel_process->threads);
    VECTOR_ALLOC(kernel_process->children);
    kernel_process->pmap = get_kernel_pagemap();
}