#include <arch/cpu.h>
#include <arch/framebuffer.h>
#include <arch/terminal.h>
#include <arch/lock.h>
#include <devices/serial.h>
#include <libc/print.h>
#include <stdarg.h>

#define WRITE_SERIAL(str) {                 \
    size_t len = __strlen(str);             \
    for (size_t i = 0; i < len; i++) {      \
        write_serial(str[i], COM1);         \
    }                                       \
    write_serial('\n', COM1);               \
}                                           \

// static volatile struct limine_terminal_request terminal_request = {
//     .id = LIMINE_TERMINAL_REQUEST,
//     .revision = 0
// };

// static struct limine_terminal *terminal;
static spinlock_t kprint_lock = 0;
static terminal kterminal;

enum TEXT_TYPE {
    TEXT_RESET,
    TEXT_BOLD,
    TEXT_FAINT,
    TEXT_ITALIC,
    TEXT_UNDERLINE,
    TEXT_RESERVED,
    TEXT_BLINKING,
    TEXT_INVERSE,
    TEXT_HIDDEN,
    TEXT_STRIKETHROUGH
};

void init_kterminal() {
    // if (terminal_request.response == NULL
    //  || terminal_request.response->terminal_count < 1) {
    //     done();
    // }

    // terminal = terminal_request.response->terminals[0];
    kterminal.is_ansi_state = 0;
}

void kerror(const char* err) {
    kprintf(LIGHT_RED"%s", err);
    done();
}

static void terminal_putchar(const char chr) {
    // terminal_request.response->write(terminal, &chr, 1);
    putchar(chr);
    // write_serial(chr, COM1);
}

static void parse_ansi_color(terminal* terminal) {

}

static void parse_sgr(terminal* terminal, char* sequence) {
    if (__strlen(sequence) == 0) {
        // Reset ANSI attributes
        return;
    }

    char *tok = __strtok(sequence, ";");
    while (tok != NULL)
    {
        int n = __atoi((const char**)&tok);

        switch (n) {
            
        }

        tok = __strtok(NULL, ";");
    }
}

static void terminal_parse_ansi(terminal* terminal) {
    char* ansi_sequence = terminal->ansi_sequence;

    if (ansi_sequence[0] != '[') {
        return;
    }

    ansi_sequence++;
    size_t code_idx = __strlen(ansi_sequence) - 1;
    char ansi_code = ansi_sequence[__strlen(ansi_sequence) - 1];
    ansi_sequence[code_idx] = '\0';

    switch (ansi_code) {
    case 'm': // Parse color code
        parse_sgr(terminal, ansi_sequence);
        break;
    }
}

static void terminal_printf(terminal* terminal, const char* buf) {
    const char* start = buf;
    
    while (*buf) {
        if (terminal->is_ansi_state) {
            char next_ansi[2] = {*buf, '\0'};
            __strcat(terminal->ansi_sequence, next_ansi);

            if (terminal->ansi_sequence[ANSI_SEQ_LEN-1] != '\0') {
                terminal->is_ansi_state = false;
                buf = ++start;
            } else if (__strchr("[;01234567890", *buf) == NULL) {
                terminal->is_ansi_state = false;
                terminal_parse_ansi(terminal);
                terminal->ansi_sequence[0] = '\0';
            }

        } else {
            switch (*buf) {
            case '\b':
                break;
            case '\033':
                terminal->is_ansi_state = true;
                break;
            case '\n':
                newline();
                break;
            case '\r':
                break;
            case '\t':
                break;
            case '\v':
                break;
            default:
                putchar(*buf);
                break;
            }
        }

        buf++;
    }
}

int kprintf(const char* format, ...) {
    char buf[BUF_MAX];
    va_list valist;
    
    spinlock_acquire(&kprint_lock);

    va_start(valist, format);
    int err = __vsnprintf(buf, BUF_MAX, format, valist);
    va_end(valist);
    ASSERT(err != -1);

    terminal_printf(&kterminal, buf);

    spinlock_release(&kprint_lock);

    return err;
}