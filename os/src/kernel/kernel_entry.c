#include <stdint.h>
#include <init.h>
#include <timer.h>
#include <kernel.h>

void kernel_entry(uintptr_t hart_id, const void* fdt_blob) {
    init(fdt_blob);
    timer_init_ticks_10ms();
    kernel_main();
}