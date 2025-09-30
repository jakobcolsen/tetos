#include <fdt_parser.h>
#include <uart.h>

// Global for UART
static volatile uintptr_t g_uart_base = 0;   // filled by FDT
static uint64_t g_uart_size = 0;   // (not used yet, but handy to keep)
static const char* g_uart_path = 0;   // e.g., "/soc/uart@10000000"
static const char* g_uart_compat = 0; // e.g., "ns16550a"


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
        uart_puts(g_uart_base, "BOOT: fdt_init failed, using hardcoded UART @0x10000000\n");
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
        uart_puts(g_uart_base, "BOOT: stdout UART not found in FDT; fallback @0x10000000\n");
        return;
    }

    // Bring up UART
    g_uart_base   = (uintptr_t) base;
    g_uart_size   = size;
    g_uart_path   = node_path;
    g_uart_compat = compatible ? compatible : "";

    uart_init(g_uart_base);

    // TetOS IS ALIVE
    uart_puts(g_uart_base, "BOOT: TetOS SBI early console ready\n");
    uart_puts(g_uart_base, "Baguette crumbs of a new OS...\n");
    uart_puts(g_uart_base, "Using UART abs_path=");
    uart_puts(g_uart_base, g_uart_path);
    uart_puts(g_uart_base, "\n");
}