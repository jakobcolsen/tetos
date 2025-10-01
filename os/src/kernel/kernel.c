#include <kernel.h>

int kernel_main(void) {
     // TetOS IS ALIVE
    kprintf("Baguette crumbs of a new OS...\n");
    kprintf("Test kprintf: char '%c', string \"%s\", int %d, uint %u, hex 0x%x, percent %%, nothing %t\n",
            'A', "Stringing a chorus out of tune!", -1234, 5678u, 0x9abc);

    // Echo
    kprintf("Type characters to echo:\n");
    kprintf("tetos> ");
    char input[128];
    while (1) {
        uart_gets(input, sizeof(input));
        kprintf("\nYou typed: %s\n", input);
        kprintf("tetos> ");
    }

    return 0;
}