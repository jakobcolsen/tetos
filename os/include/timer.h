#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <sbi.h>

extern volatile uint32_t g_timer_hz; // Global timer frequency in Hz

void timer_init(uint32_t hz);

void sleep_ms(uint64_t ms);

#endif // TIMER_H