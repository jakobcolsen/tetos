#include <stdint.h>
#include <stddef.h>

static inline void putc_hard(char c) {
    volatile uint8_t *lsr = (uint8_t*) (uintptr_t) (0x10000000ull + 5);
    volatile uint8_t *thr = (uint8_t*) (uintptr_t)( 0x10000000ull + 0);
    while ((*lsr & (1u<<5)) == 0) { }   // wait for THR empty (LSR bit5)
    *thr = (uint8_t)c;
}

static inline void puts_hard(const char *s) { while (*s) putc_hard(*s++); }

void sbi_init(const void *fdt) {
    puts_hard("BOOT\n");

    return;
}