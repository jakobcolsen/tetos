#include <fdt_parser.h>
#include <kprintf.h>
#include <panic.h>

void sbi_init(const void* fdt_blob) {
    // Build a view of the FDT
    FDTView_t view;
    uint32_t totalsize = 0;

    if (fdt_blob) {
        // The header is packed; totalsize is big-endian at offset 4
        totalsize = read_be32((const uint8_t*) fdt_blob + 4);
    }

    size_t blob_size = (size_t) totalsize;

    if (fdt_init(&view, fdt_blob, blob_size) != 0) {
        // We don't have UART yet; use QEMU's default 0x10000000 as a last resort
        g_uart_base = 0x10000000ull;
        uart_init(g_uart_base);
        panic("BOOT: fdt_init failed!");
        return;
    }

    // Resolve /chosen -> stdout-path -> UART node
    uint64_t base = 0;
    uint64_t size = 0;
    const char* node_path = NULL;
    const char* compatible = NULL;

    int rc = fdt_resolve_stdout_uart(&view, &base, &size, &node_path, &compatible);
    if (rc != 0 || base == 0) {
        // Fallback: common QEMU virt mapping
        g_uart_base = 0x10000000ull;
        uart_init(g_uart_base);
        panic("BOOT: stdout UART not found in FDT!");
        return;
    }

    // Bring up UART
    g_uart_base   = (uintptr_t) base;
    uart_init(g_uart_base);

    // TetOS IS ALIVE
    kprintf("BOOT: FDT UART FOUND! @0x%x!\n", g_uart_base);
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
}