#ifndef HPET_H
#define HPET_H

#include <stdint.h>

#define HPET_GENERAL_CAP_AND_ID             0x00
#define HPET_GENERAL_CONFIG                 0x10
#define HPET_MAIN_COUNTER                   0xF0

void hpet_write(uint32_t offset, uint64_t value);
uint64_t hpet_read(uint32_t offset);
void hpet_timer_enable(void);
void hpet_timer_disable(void);
void init_hpet(void);

#endif // HPET_H