#include <limine.h>
#include <stddef.h>
#include <stdarg.h>
#include <libc/string.h>
#include <libc/ctype.h>
#include <libc/stdlib.h>

// Resets state of colors
#define RESET           "\033[m"

// Terminal text colors
#define BLACK           RESET"\033[30m"
#define RED             RESET"\033[31m"
#define GREEN           RESET"\033[32m"
#define YELLOW          RESET"\033[33m"
#define BLUE            RESET"\033[34m"
#define MAGENTA         RESET"\033[35m"
#define CYAN            RESET"\033[36m"
#define LIGHT_GRAY      RESET"\033[37m"
#define GRAY            RESET"\033[1;30m"
#define LIGHT_RED       RESET"\033[1;31m"
#define LIGHT_GREEN     RESET"\033[1;32m"
#define LIGHT_YELLOW    RESET"\033[1;33m"
#define LIGHT_BLUE      RESET"\033[1;34m"
#define LIGHT_MAGENTA   RESET"\033[1;35m"
#define LIGHT_CYAN      RESET"\033[1;36m"
#define WHITE           RESET"\033[1;37m"

void init_terminal(void);
void done(void);

void terminal_putchar(const char chr);
int terminal_vfprintf(va_list valist, const char* format);
int terminal_printf(const char* format, ...);
