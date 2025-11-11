#include <stdint.h>
#include <sbi.h>
#include <panic.h>

#define SCAUSE_INT_MASK (1ULL << 63)
#define SCAUSE_CODE(x) ((x) & 0xFFFULL)

#define SIE_SSIE (1ULL << 1)
#define SIE_STIE (1ULL << 5)
#define SIE_SEIE (1ULL << 9)
#define SSTATUS_SIE (1ULL << 1)

static volatile uint64_t g_ticks;

static inline void enable_sie_bits(uint64_t bits) {
    asm volatile("csrs sie, %0" :: "r"(bits) : "memory");
}
static inline void enable_sstatus_sie(void) {
    asm volatile("csrs sstatus, %0" :: "r"(SSTATUS_SIE) : "memory");
}

void trap_entry(uint64_t scause, uint64_t sepc, uint64_t stval) {
    if (scause & SCAUSE_INT_MASK) {
        switch (SCAUSE_CODE(scause)) {
        case 5: { // STIP: supervisor timer interrupt
            g_ticks++;
            // QEMU virt timebase is 10 MHz -> 10 ms = 100,000 ticks
            uint64_t now  = rdtime();
            uint64_t next = now + 100000ULL; // 10 ms
            sbi_set_timer(next);
            return; 
        }
        case 1:
            // SSIP (IPI) — later: clear SSIP bit
            // asm volatile("csrc sip, %0" :: "r"(1ULL<<1) : "memory");
            return;
        case 9:
            // SEIP (PLIC) — later: claim/complete in PLIC
            return;
        default:
            break;
        }
    }

    // Unexpected exception
    sbi_console_putc('X');
    sbi_console_putc('\n');
    panic("Unhandled trap");
}
