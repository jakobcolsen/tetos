#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <sbi.h>

void timer_init_ticks_10ms();
void sleep_ms(uint64_t ms);

#endif // TIMER_H