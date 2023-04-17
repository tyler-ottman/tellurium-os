#ifndef PIT_H
#define PIT_H

#include <stdint.h>

void pit_set_reload(uint8_t channel, uint16_t value);
uint16_t pit_get_count(uint8_t channel);
void init_pit(void);

#endif // PIT_H