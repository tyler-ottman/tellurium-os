#include <limine.h>
#include <stddef.h>
#include <stdarg.h>
#include <libc/string.h>
#include <libc/ctype.h>
#include <libc/stdlib.h>

void init_terminal(void);
void done(void);

void terminal_putchar(const char chr);
int terminal_vfprintf(va_list valist, const char* format);
int terminal_printf(const char* format, ...);
