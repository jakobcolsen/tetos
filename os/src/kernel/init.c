#include <init.h>

void init(const void* fdt_blob) {
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
        g_uart_base = UART_DEFAULT_MAP;
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
        g_uart_base = UART_DEFAULT_MAP;
        uart_init(g_uart_base);
        panic("BOOT: stdout UART not found in FDT!");
        return;
    }

    // Bring up UART
    g_uart_base   = (uintptr_t) base;
    uart_init(g_uart_base);
}