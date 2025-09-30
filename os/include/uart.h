#include <stdint.h>
#include <stddef.h>

#ifndef UART_H
#define UART_H

// This is a big one:
// __attribute__((packed)) ensures no padding is added by the compiler, this is necessary
// because we are overlaying this struct on top of memory-mapped hardware registers.

// Union is used for registers that share the same address, depending on context.
// NS16550 has DLAB bit in LCR to switch between RBR/THR and DLL/DLH.

// https://bitsavers.trailing-edge.com/components/national/_appNotes/AN-0491.pdf
typedef struct __attribute__((packed)) {
    union { volatile uint8_t RBR; volatile uint8_t THR; volatile uint8_t DLL; }; // 0
    union { volatile uint8_t IER; volatile uint8_t DLH; }; // 1
    union { volatile uint8_t IIR; volatile uint8_t FCR; }; // 2
    volatile uint8_t LCR; // 3
    volatile uint8_t MCR; // 4
    volatile uint8_t LSR; // 5
    volatile uint8_t MSR; // 6
    volatile uint8_t SCR; // 7
} ns16550_8_t;

#define UART(base) ((ns16550_8_t*) (uintptr_t) (base)) // Cast base address to struct pointer

void uart_init(uintptr_t base);
void uart_putc(uintptr_t base, char c);
void uart_puts(uintptr_t base, const char* str);

#endif // UART_H
