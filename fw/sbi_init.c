#include "include/fdt_parser.h"

// Global for UART
static volatile uintptr_t g_uart_base = 0;   // filled by FDT
static uint64_t g_uart_size = 0;   // (not used yet, but handy to keep)
static const char* g_uart_path = 0;   // e.g., "/soc/uart@10000000"
static const char* g_uart_compat = 0; // e.g., "ns16550a"

// This is a big one:
// __attribute__((packed)) ensures no padding is added by the compiler, this is necessary
// because we are overlaying this struct on top of memory-mapped hardware registers.

// Union is used for registers that share the same address, depending on context.
// NS16550 has DLAB bit in LCR to switch between RBR/THR and DLL/DLH.

// https://bitsavers.trailing-edge.com/components/national/_appNotes/AN-0491.pdf
typedef struct __attribute__((packed)) {
    union { volatile uint8_t RBR; volatile uint8_t THR; volatile uint8_t DLL; }; // 0
    union { volatile uint8_t IER; volatile uint8_t DLH; }; // 1
    union { volatile uint8_t IIR; volatile uint8_t FCR; }; // 2
    volatile uint8_t LCR; // 3
    volatile uint8_t MCR; // 4
    volatile uint8_t LSR; // 5
    volatile uint8_t MSR; // 6
    volatile uint8_t SCR; // 7
} ns16550_8_t;

#define UART(base) ((ns16550_8_t*) (uintptr_t) (base)) // Cast base address to struct pointer
static inline void uart_init(uintptr_t base) {
    ns16550_8_t* uart = UART(base);
    uart->IER = 0x00; // Disable all interrupts (polling for boot)
    uart->FCR = 0x07; // Enable FIFO, clear RX/TX queues
    uart->LCR = 0x03; // 8 bits, no parity, one stop bit
    uart->MCR = 0x03; // RTS/DSR set
}

static inline void uart_putc(uintptr_t base, char c) {
    ns16550_8_t* uart = UART(base);
    while ((uart->LSR & (1 << 5)) == 0); // Wait for THR empty
    uart->THR = (uint8_t) c;
}

static inline void uart_puts(uintptr_t base, const char* str) {
    while (*str) {
        if (*str == '\n') {
            uart_putc(base, '\r'); // Carriage return before newline
        }
        uart_putc(base, *str++);
    }
}

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