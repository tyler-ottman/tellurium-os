#include <arch/terminal.h>

// The Limine requests can be placed anywhere, but it is important that
// the compiler does not optimise them away, so, usually, they should
// be made volatile or equivalent.
static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

static struct limine_terminal *terminal;

void init_terminal() {
    if (terminal_request.response == NULL
     || terminal_request.response->terminal_count < 1) {
        done();
    }

    terminal = terminal_request.response->terminals[0];
}

// Halt CPU activity
void done(void) {
    for (;;) {
        __asm__("hlt");
    }
}

void terminal_putchar(const char chr) {
    terminal_request.response->write(terminal, &chr, 1);
}

#define PRINTF_PAD_ZERO_MASK        0

int terminal_vfprintf(va_list valist, const char* format) {
    uint64_t flags; // Flags in format specifier
    uint64_t width; // Field width in format specifier
    uint64_t base; // Default base for numbers

    // Push default color
    terminal_request.response->write(terminal, CYAN, __strlen(CYAN));

    while (*format) {
        flags = 0;
        width = 0;
        base = 0;

        if (*format != '%') {
            // Not format specifier, write character to terminal
            terminal_putchar(*format);
            format++;
            continue;
        }

        // If here then check format specifier
        format++;

        // Check flags
        switch (*format) {
            case '0':
                flags |= (1 << PRINTF_PAD_ZERO_MASK);
                format++;
                break;
            default: break; // Todo: more features
        }

        // Check width of field
        if (__is_digit(*format)) {
            // Atoi here increments the format pointer so it
            // points passed the field width characters
            width = __atoi(&format);
        }

        switch (*format) {
        case 'i':
        case 'd':
        case 'b':
        case 'o':
        case 'u':
        case 'x': {

            // Base used when converting number to string
            switch (*format) {
            case 'b': base = 2; break;
            case 'o': base = 8; break;
            case 'u': base = 10; break;
            case 'x': base = 16; break;
            }

            // Assume 64-bit for now
            if ((*format == 'i') || (*format == 'd')) { // Signed Integer
                // Todo, signed
                uint64_t val = va_arg(valist, long long);\

                // Get string representation of number
                char str_num[256];
                __itoa(val, str_num);

                // Calculate padding taking into account length of string
                uint64_t len = __strlen(str_num);
                if ((len >= width) || (width == 0)) {
                    // No padding needed
                    width = 0;
                } else {
                    width -= len;
                }

                // Padding character either '0' or ' '
                char padding_char = (flags & 1) ? '0' : ' ';

                // Apply padding
                for (uint64_t idx = 0; idx < width; idx++) {
                    terminal_putchar(padding_char);
                }

                // Print string representation of number
                char* str = str_num;
                while (*str) {
                    terminal_putchar(*str);
                    str++;
                }

            } else { // Unsigned Binary/Octal/Decimal/Hex
                uint64_t val = va_arg(valist, long long);
                
                // Get string representation of number
                char str_num[256];
                __utoa(val, str_num, base);

                // Calculate padding taking into account length of string
                uint64_t len = __strlen(str_num);
                if ((len >= width) || (width == 0)) {
                    // No padding needed
                    width = 0;
                } else {
                    width -= len;
                }

                // Padding character either '0' or ' '
                char padding_char = (flags & 1) ? '0' : ' ';

                // Apply padding
                for (uint64_t idx = 0; idx < width; idx++) {
                    terminal_putchar(padding_char);
                }

                // Print string representation of number
                char* str = str_num;
                while (*str) {
                    terminal_putchar(*str);
                    str++;
                }
            }
            format++;
            break;
        }
        case 'c': {
            if (width != 0) { // Pad when arguments given
                uint64_t zero_pad = 0;
                while (zero_pad++ < (width - 1)) {
                    terminal_putchar(' ');
                }
            }

            terminal_putchar((char)va_arg(valist, int));
            
            format++;
            break;
        }
        case 's': {
            // Print string argument
            const char *arg = va_arg(valist, char *);
            
            // Calculate padding taking into account length of string
            uint64_t len = __strlen(arg);
            if ((len >= width) || (width == 0)) {
                // No padding needed
                width = 0;
            } else {
                // Subtract length of string from field width
                width -= len;
            }

            // Apply padding
            for (uint64_t idx = 0; idx < width; idx++) {
                terminal_putchar(' ');
            }

            // String argument
            while (*arg) {
                terminal_putchar(*arg);
                arg++;
            }

            format++;
            break;
        }
        default: // If character following '%' is garbage just print it
            terminal_putchar(*format);
            format++;
            break;
        }

    }

    return 1;
}

/* Basic printf function for debugging terminal in VGA text mode
Features:
    - Print string literals
        - Ex: terminal_printf("Hello World!");
    - Print formatted character with or without LEFT padding
        - Ex: terminal_printf("Hel%5clo World!", 'f');
        - Ex: terminal_printf("Hel%clo World!", 'f');
    - Print formatted strings with or without LEFT padding
        - Ex: terminal_printf("Hello %s\n", "World");
        - Ex: terminal_printf("Hello %20s\n", "World");
    - Print formatted base 2/8/10/16 number with or without LEFT '0'/' ' padding
        - Ex: terminal_printf("Hello %b\n", 18);
        - Ex: terminal_printf("Hello %o\n", 18);
        - Ex: terminal_printf("Hello %u\n", 18);
        - Ex: terminal_printf("Hello %x\n", 18);
        - Ex: terminal_printf("Hello %8b\n", 18);
        - Ex: terminal_printf("Hello %8o\n", 18);
        - Ex: terminal_printf("Hello %8u\n", 18);
        - Ex: terminal_printf("Hello %8x\n", 18);
        - Ex: terminal_printf("Hello %08b\n", 18);
        - Ex: terminal_printf("Hello %08o\n", 18);
        - Ex: terminal_printf("Hello %08u\n", 18);
        - Ex: terminal_printf("Hello %08x\n", 18);
*/
int terminal_printf(const char* format, ...) {
    va_list valist;
    int err;
    
    va_start(valist, format);
    err = terminal_vfprintf(valist, format);
    va_end(valist);

    return err;
}