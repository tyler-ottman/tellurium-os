#ifndef CPU_H
#define CPU_H

#include <acpi/acpi.h>
#include <arch/gdt.h>
#include <arch/idt.h>
#include <arch/terminal.h>
#include <devices/lapic.h>
#include <devices/serial.h>
#include <limine.h>
#include <memory/vmm.h>

void init_cpu(void);
void core_init(struct limine_smp_info* core);

#endif // CPU_H