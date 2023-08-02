#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <arch/cpu.h>
#include <arch/kterminal.h>
#include <flibc/string.h>
#include <stdint.h>

void exception_handler(uint8_t vector);
void exception_handler_err(uint8_t vector, ctx_t *ctx);

#endif // EXCEPTION_H