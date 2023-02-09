#ifndef HPET_H
#define HPET_H

#include <stdint.h>

#define HPET_GENERAL_CAP_AND_ID             0x00
#define HPET_GENERAL_CONFIG                 0x10
#define HPET_MAIN_COUNTER                   0xF0

#define FEMTO                               1000000000000000

uint32_t hpet_get_period(void);
uint32_t hpet_get_timer(void);
void hpet_set_timer(uint64_t val);
void hpet_timer_enable(void);
void hpet_timer_disable(void);
void init_hpet(void);

#endif // HPET_H