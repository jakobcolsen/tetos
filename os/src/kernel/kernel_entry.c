#include <stdint.h>
#include <init.h>
#include <kernel.h>
#include <kprintf.h>

void kernel_entry(uintptr_t hart_id, const void* fdt_blob) {
    init(fdt_blob);
    kprintf("Kernel initialized on hart %lu\n", hart_id);
    kernel_main();
}