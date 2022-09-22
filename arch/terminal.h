#include <limine.h>
#include <stddef.h>
#include <libc/string.h>

void init_terminal(void);
void done(void);

void terminal_print(const char* str);
void terminal_println(const char* str);
