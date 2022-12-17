#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <arch/thread.h>
#include <arch/process.h>

struct tcb* create_kernel_thread(void* entry, void* param);

#endif // SCHEDULER_H
