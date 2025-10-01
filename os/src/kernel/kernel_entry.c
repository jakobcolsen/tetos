#include <sbi_init.h>
#include <kernel.h>

// Pretty basic, probably want to do something more useful here
void kernel_entry(const void* fdt_blob) {
    sbi_init(fdt_blob);

    kernel_main();
}