#include <kernel.h>

int kernel_main(void) {
     // TetOS IS ALIVE
    kprintf("Baguette crumbs of a new OS...\n");
    kernel_monitor(); 
    return 0;
}