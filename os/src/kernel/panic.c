#include <panic.h>

void _panic(const char* msg, const char* file, int line, const char* func) {
    kprintf("\n*** KERNEL PANIC ***\n");
    kprintf("Message: %s\n", msg);
    kprintf("Location: %s:%d\n", file, line);
    kprintf("Function: %s\n", func);

    while (1) {
        __asm__ volatile ("wfi"); // Hang
    }
}