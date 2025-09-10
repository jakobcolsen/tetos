#include <stdint.h>
#include <stddef.h>
#include "include/fdt_parser.h"

static inline void putc_hard(char c) {
    volatile uint8_t *lsr = (uint8_t*) (uintptr_t) (0x10000000ull + 5);
    volatile uint8_t *thr = (uint8_t*) (uintptr_t)( 0x10000000ull + 0);
    while ((*lsr & (1u<<5)) == 0) { }   // wait for THR empty (LSR bit5)
    *thr = (uint8_t)c;
}

static inline void puts_hard(const char *s) { while (*s) putc_hard(*s++); }

void sbi_init(const void *fdt) {
    puts_hard("BOOT\n");

    // Print out FDT structure for testing
    FDTView_t fdt_view;
    int res = fdt_init(&fdt_view, fdt, 0x10000); // Assume max size of 64KB for now
    if (res != 0) {
        puts_hard("FDT init failed\n");
        return;
    }

    FDTCursor_t cursor = {
        .current = fdt_view.struct_begin,
        .end = fdt_view.struct_end
    };

    int output = 0;
    int tabs = 0;
    while (1) {
        FDTToken_t token;
        const char* name = NULL;
        FDTProp_t prop;
        output = fdt_next(&cursor, &fdt_view, &token, &name, &prop);
        if (output != 0) break;

        if (token == FDT_BEGIN_NODE) tabs++;
        for (int i = 0; i < tabs; i++) putc_hard('\t'); // Indent based on depth

        switch (token) {
            case FDT_BEGIN_NODE:
                puts_hard("BEGIN NODE: ");
                puts_hard(name ? name : "(null)");
                puts_hard("\n");
                break;

            case FDT_END_NODE:
                puts_hard("END NODE\n");
                tabs--;
                break;

            case FDT_PROP:
                puts_hard("PROP: ");
                puts_hard(prop.name ? prop.name : (const char*) "(null)");
                
                puts_hard(")\n");
                break;

            case FDT_NOP:
                puts_hard("NOP\n");
                break;

            default:
                puts_hard("UNKNOWN TOKEN\n");
                break;
        }
    }
}