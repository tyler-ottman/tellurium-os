#include <stdint.h>

#define IA32_APIC_BASE      0x0000001B
#define IA32_EFER           0xC0000080
#define IA32_STAR           0xC0000081
#define IA32_LSTAR          0xC0000082
#define IA32_FMASK          0xC0000084
#define FS_BASE             0xC0000100
#define GS_BASE             0xC0000101
#define KERNEL_GS_BASE      0xC0000102

uint64_t get_msr(uint32_t msr);
void set_msr(uint32_t msr, uint64_t val);