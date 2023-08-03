#include <arch/cpu.h>
#include <arch/kterminal.h>
#include <arch/limine_fb.h>
#include <arch/lock.h>
#include <flibc/print.h>
#include <klib/kmalloc.h>

spinlock_t kprint_lock = 0;
terminal_t kterminal;

terminal_t *get_kterminal() {
    return &kterminal;
}

void kerror(const char* msg, int err) {
    disable_interrupts();

    const char msg_default[] = "error";

    kprintf(ERROR "%s: %d\n", msg ? msg : msg_default, err);
    
    core_hlt();
}

int kprintf(const char *format, ...) {
    char buf[512];
    va_list valist;

    va_start(valist, format);
    __vsnprintf(buf, BUF_MAX, format, valist);
    va_end(valist);

    int state = core_get_if_flag();
    
    disable_interrupts();

    spinlock_acquire(&kprint_lock);

    kterminal.apply_set_attribute[0](&kterminal);
    kterminal.print(&kterminal, buf);

    // Effective screen refresh
    kterminal.refresh(&kterminal);

    spinlock_release(&kprint_lock);
    
    if (state) {
        enable_interrupts();
    }

    return 0;
}

void init_kterminal() {
    init_framebuffer();

    terminal_alloc(&kterminal, 14, 8, fb_get_height(), fb_get_width(),
                   fb_get_height(), fb_get_width(), fb_get_pitch(),
                   fb_get_bpp(), FG_COLOR_DEFAULT, BG_COLOR_DEFAULT, NULL, NULL,
                   fb_get_framebuffer(), kmalloc, kfree);
}