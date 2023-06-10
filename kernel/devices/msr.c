#include <devices/msr.h>

uint64_t get_msr(uint32_t msr) {
    uint32_t low, high;
    __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr) : "memory");
    return ((uint64_t)high << 32) | low;
}

void set_msr(uint32_t msr, uint64_t val) {
    uint32_t low = val & 0xffffffff;
    uint32_t high = (val >> 32) & 0xffffffff;

    __asm__ volatile ("wrmsr" : : "a"(low), "d"(high), "c"(msr) : "memory");
}