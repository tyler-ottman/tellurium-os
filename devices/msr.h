#include <stdint.h>

#define IA32_APIC_BASE 0x1B

uint64_t get_msr(uint32_t msr);
void set_msr(uint32_t msr, uint64_t val);