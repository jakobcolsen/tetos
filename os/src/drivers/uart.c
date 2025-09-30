#include <uart.h>

void uart_init(uintptr_t base) {
    ns16550_8_t* uart = UART(base);
    uart->IER = 0x00; // Disable all interrupts (polling for boot)
    uart->FCR = 0x07; // Enable FIFO, clear RX/TX queues
    uart->LCR = 0x03; // 8 bits, no parity, one stop bit
    uart->MCR = 0x03; // RTS/DSR set
}

void uart_putc(uintptr_t base, char c) {
    ns16550_8_t* uart = UART(base);
    while ((uart->LSR & (1 << 5)) == 0); // Wait for THR empty
    uart->THR = (uint8_t) c;
}

void uart_puts(uintptr_t base, const char* str) {
    while (*str) {
        if (*str == '\n') {
            uart_putc(base, '\r'); // Carriage return before newline
        }
        uart_putc(base, *str++);
    }
}
