#include <stdint.h>

extern uint64_t _sbss;
extern uint64_t _ebss;

extern int main(void);

extern "C" void _start(void) {
    uint64_t *base = &_sbss;
    uint64_t *end = &_ebss;
    while (base < end) {
        *base++ = 0;
    }

    main();
}