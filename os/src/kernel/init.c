#include <init.h>

static void uart_fdt(const FDTView_t* fdt) {
    uint64_t base = 0;
    uint64_t size = 0;
    const char* node_path = NULL;
    const char* compatible = NULL;

    int rc = fdt_resolve_stdout_uart(fdt, &base, &size, &node_path, &compatible);
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

static void timer_fdt(const FDTView_t* fdt) {
    uint32_t hz = 0;
    const char* src_path = NULL;

    int rc = fdt_read_timebase_frequency(fdt, &hz, &src_path);
    if (rc != 0 || hz == 0) {
        // Fallback: common QEMU virt frequency
        hz = 10000000; // 10 MHz
        panic("BOOT: timebase-frequency not found in FDT!");
    }

    timer_init(hz);
}

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
        panic("BOOT: Failed to initialize FDT view!");
        return;
    }

    uart_fdt(&view);
    timer_fdt(&view);
}