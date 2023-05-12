#include <arch/cpu.h>
#include <libc/ctype.h>
#include <libc/print.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/misc.h>

#define ASSERT_BUFF_LIM(cur, max) {if (cur >= max) return -1;} 

int __vsnprintf(char* buf, size_t n, const char* format, va_list valist) {
    uint64_t flags; // Flags in format specifier
    uint64_t width; // Field width in format specifier
    uint64_t base; // Default base for numbers
    uint64_t cur_idx = 0;
    char str_num[256];

    while (*format) {
        flags = 0;
        width = 0;
        base = 0;

        if (*format != '%') {
            // Not format specifier, write character to buffer
            ASSERT_BUFF_LIM(cur_idx, n);
            buf[cur_idx++] = *format;
            format++;
            continue;
        }

        // If here then check format specifier
        format++;

        // Check flags
        switch (*format) {
            case '0':
                flags |= 1;
                format++;
                break;
            default: break; // Todo: more features
        }

        // Check width of field
        if (__is_digit(*format)) {
            width = __atoi(format);

            int temp_width = width;
            while (temp_width != 0) {
                temp_width /= 10;
                format++;
            }
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
                uint64_t val = va_arg(valist, uint64_t);

                // Get string representation of number
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
                    ASSERT_BUFF_LIM(cur_idx, n);
                    buf[cur_idx++] = padding_char;
                }

                // Print string representation of number
                char* str = str_num;
                while (*str) {
                    ASSERT_BUFF_LIM(cur_idx, n);
                    buf[cur_idx++] = *str;
                    str++;
                }

            } else { // Unsigned Binary/Octal/Decimal/Hex
                uint64_t val = va_arg(valist, long long);

                // Get string representation of number
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
                    ASSERT_BUFF_LIM(cur_idx, n);
                    buf[cur_idx++] = padding_char;
                }

                // Print string representation of number
                char* str = str_num;
                while (*str) {
                    ASSERT_BUFF_LIM(cur_idx, n);
                    buf[cur_idx++] = *str;
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
                    ASSERT_BUFF_LIM(cur_idx, n);
                    buf[cur_idx++] = ' ';
                }
            }

            ASSERT_BUFF_LIM(cur_idx, n);
            buf[cur_idx++] = (char)va_arg(valist, int);
            
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
                ASSERT_BUFF_LIM(cur_idx, n);
                buf[cur_idx++] = ' ';
            }

            // String argument
            while (*arg) {
                ASSERT_BUFF_LIM(cur_idx, n);
                buf[cur_idx++] = *arg;
                arg++;
            }

            format++;
            break;
        }
        default: // If character following '%' is garbage just print it
            ASSERT_BUFF_LIM(cur_idx, n);
            buf[cur_idx++] = *format;
            format++;
            break;
        }

    }

    ASSERT_BUFF_LIM(cur_idx, n);
    buf[cur_idx++] = '\0';

    return cur_idx;
}

int __snprintf(char* buf, size_t n, const char* format, ...) {
    va_list valist;
    int err;
    
    va_start(valist, format);
    err = __vsnprintf(buf, n, format, valist);
    va_end(valist);

    return err;
}