#include <arch/cpu.h>
#include <stdint.h>

#define STACK_CHK_GUARD 0x595e9fbd94fda766
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void) {
    disable_interrupts();
    core_hlt();
}
