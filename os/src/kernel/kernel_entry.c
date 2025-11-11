#include <stdint.h>
#include <init.h>
#include <kernel.h>

void kernel_entry(uintptr_t hart_id, const void* fdt_blob) {
    init(fdt_blob);
    kernel_main();
}