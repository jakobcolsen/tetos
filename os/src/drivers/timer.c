#include <timer.h>

#define SIE_STIE (1ULL << 5)
#define SSTATUS_SIE (1ULL << 1)
#define TIMER_10MS_TICKS 100000ULL // 10ms at 10MHz

void timer_init_ticks_10ms() {
    const uint64_t now = rdtime();
    sbi_set_timer(now + TIMER_10MS_TICKS);

    // enable STIP and global SIE
    asm volatile("csrs sie, %0" :: "r"(SIE_STIE));
    asm volatile("csrs sstatus, %0" :: "r"(SSTATUS_SIE));
}

void sleep_ms(uint64_t ms) {
    // wfi for now, set timer for ms in future
    const uint64_t now = rdtime();
    sbi_set_timer(now + ms * TIMER_10MS_TICKS);

    // ensure interrupts enabled
    asm volatile("csrs sie, %0" :: "r"(SIE_STIE));
    asm volatile("csrs sstatus, %0" :: "r"(SSTATUS_SIE));

    asm volatile("wfi"); // returns on next timer interrupt
}
