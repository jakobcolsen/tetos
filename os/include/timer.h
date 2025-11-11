#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <sbi.h>

static volatile uint32_t g_timer_hz = 0; // Global timer frequency in Hz

void timer_init(uint32_t hz);
void timer_init_ticks_10ms();
void sleep_ms(uint64_t ms);

#endif // TIMER_H