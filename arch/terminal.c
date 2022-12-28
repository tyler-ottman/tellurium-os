#include <arch/framebuffer.h>
#include <arch/terminal.h>
#include <arch/lock.h>
#include <libc/print.h>
#include <stdarg.h>

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.
static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

static struct limine_terminal *terminal;
static spinlock_t kprint_lock = 0;

void init_terminal() {
    if (terminal_request.response == NULL
     || terminal_request.response->terminal_count < 1) {
        done();
    }

    terminal = terminal_request.response->terminals[0];
}

void kerror(const char* err) {
    kprintf(LIGHT_RED"%s", err);
    done();
}

void terminal_putchar(const char chr) {
    // terminal_request.response->write(terminal, &chr, 1);
    putchar(chr);
    // write_serial(chr, COM1);
}

#define BUF_MAX     512
int kprintf(const char* format, ...) {
    char buf[BUF_MAX];
    va_list valist;
    
    spinlock_acquire(&kprint_lock);

    va_start(valist, format);
    int err = __vsnprintf(buf, BUF_MAX, format, valist);
    va_end(valist);
    ASSERT(err != -1);

    for (size_t i = 0; buf[i]; i++) {
        terminal_putchar(buf[i]);
    }

    spinlock_release(&kprint_lock);

    return err;
}