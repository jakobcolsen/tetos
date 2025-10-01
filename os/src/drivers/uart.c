#include <uart.h>

void uart_init(uintptr_t base) {
    g_uart_base = base;
    ns16550_8_t* uart = UART(base);
    uart->IER = 0x00; // Disable all interrupts (polling for boot)
    uart->FCR = 0x07; // Enable FIFO, clear RX/TX queues
    uart->LCR = 0x03; // 8 bits, no parity, one stop bit
    uart->MCR = 0x03; // RTS/DSR set
}

void uart_putc(char c) {
    ns16550_8_t* uart = UART(g_uart_base);
    while ((uart->LSR & (1 << 5)) == 0); // Wait for THR empty
    uart->THR = (uint8_t) c;
}

void uart_puts(const char* str) {
    while (*str) {
        if (*str == '\n') {
            uart_putc('\r'); // Carriage return before newline
        }
        uart_putc(*str++);
    }
}

char uart_getc(void) {
    ns16550_8_t* uart = UART(g_uart_base);
    while ((uart->LSR & 0x01) == 0); // Wait for data available (LSR[0] = 1)
    return (char) (uart->RBR);
}

void uart_gets(char* buffer, size_t max_length) {
    if (max_length == 0) return; // No space to store anything

    size_t i = 0;
    while (i < (max_length - 1)) {
        char c = uart_getc();

        if (c == 0x1b) { // Escape character
            continue; // Ignore for now
        } else if (c == '\r' || c == '\n') {
            uart_puts("\r\n"); // Echo newline
            break; // Stop on enter/return
        } else if (c == '\b' || c == 127) { // Backspace or DEL
            if (i > 0) {
                i--;
                uart_puts("\b \b"); // Erase character on terminal
            }
        } else {
            uart_putc(c); // Echo
            buffer[i++] = c;
        }
    }

    buffer[i] = '\0'; // Null-terminate the string
}
