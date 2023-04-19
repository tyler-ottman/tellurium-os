#include <acpi/acpi.h>
#include <arch/cpu.h>
#include <arch/framebuffer.h>
#include <arch/gdt.h>
#include <arch/idt.h>
#include <arch/process.h>
#include <arch/terminal.h>
#include <devices/hpet.h>
#include <devices/ioapic.h>
#include <devices/lapic.h>
#include <devices/pit.h>
#include <devices/ps2.h>
#include <devices/serial.h>
#include <limine.h>
#include <memory/pmm.h>
#include <memory/slab.h>
#include <memory/vmm.h>
#include <stdint.h>
#include <stddef.h>
#include <fs/tmpfs.h>
#include <fs/vfs.h>

void init_system() {
    init_kterminal();
    init_gdt();
    init_idt();
    init_pmm();
    init_slab();
    init_kterminal_doublebuffer();
    init_acpi();
    init_hpet();
    init_ps2();
    init_pit();
    init_vmm();
    init_serial();
}

void _start(void) {
    init_system();
    init_kernel_process();

    init_cpu();

    kprintf(INFO GREEN "Kernel: Initialized\n");

    done();
}

void kmain(void *param) {
    vfs_init();

    tmpfs_init();
    
    vfs_create(vfs_get_root(), "/tmp", VDIR);
    vfs_mount(vfs_get_root(), "/tmp", "tmpfs");

    // Store userapps in vfs
    tmpfs_load_userapps();

    // For testing purposes
    create_user_process("/tmp/helloWorld.elf");

    vfs_print_tree(vfs_get_root(), 4);
    kprintf(INFO "kmain: init complete\n");
}