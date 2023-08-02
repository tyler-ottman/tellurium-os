#ifndef PRINT_H
#define PRINT_H

#include <stdarg.h>
#include <stddef.h>

int __snprintf(char *buf, size_t n, const char *format, ...);
int __vsnprintf(char *buf, size_t n, const char *format, va_list valist);

#endif // PRINT_H