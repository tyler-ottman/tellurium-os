#include <arch/cpu.h>
#include <arch/kterminal.h>
#include <arch/kterminal_font.h>
#include <arch/limine_fb.h>
#include <arch/lock.h>
#include <libc/kmalloc.h>
#include <libc/print.h>

spinlock_t kprint_lock = 0;
terminal_t kterminal;

// static void print_color_palette() {
//     for (size_t i = 0; i < 16; i++) {
//         kprintf("\033[38;5;%i;48;5;%im%03i", i, i, i);
//     }

//     kprintf("\n\n");
//     for (size_t i = 0; i < 6; i ++) {
//         for (size_t j = 0; j < 36; j++) {
//             uint8_t color_id = 16 + 36 * i + j;
//             kprintf("\033[38;5;%i;48;5;%im%03i", color_id, color_id, color_id);
//         }
//         kprintf("\n");
//     }

//     kprintf("\n");
//     for (size_t i = 232; i < 256; i++) {
//         kprintf("\033[38;5;%i;48;5;%im%03i", i, i, i);
//     }
//     kprintf("\n\n");
// }

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
    terminal_printf(&kterminal, buf);

    // Effective screen refresh
    terminal_refresh(&kterminal);

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
                   fb_get_bpp(), FG_COLOR_DEFAULT, BG_COLOR_DEFAULT,
                   kterminal_font, NULL, fb_get_framebuffer(), kmalloc, kfree);

    // print_color_palette();
}