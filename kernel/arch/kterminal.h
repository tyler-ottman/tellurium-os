#ifndef KTERMINAL_H
#define KTERMINAL_H

#include <modules/terminal.h>

terminal_t *get_kterminal(void);

void kerror(const char *msg, int err);
int kprintf(const char *format, ...);
void init_kterminal(void);

#endif // KTERMINAL_H