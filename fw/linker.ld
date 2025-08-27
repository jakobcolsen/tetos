OUTPUT_ARCH(riscv)
ENTRY(_start)
MEMORY { RAM (rxw) : ORIGIN = 0x80000000, LENGTH = 256M }

SECTIONS {
    . = ORIGIN(RAM); /* Boot address for QEMU virt */

    .text : { *(.text*) } > RAM /* Actual Code */
    .rodata : { *(.rodata) } > RAM /* Read-only data */
    .data : { *(.data*) } > RAM
    .bss : { *(.bss*) *(COMMON) } > RAM /* Unintialized variables */

    . = ORIGIN(RAM) + LENGTH(RAM);
    . = ALIGN(16); /* RISC-V stack aligns at 16. */
    PROVIDE(__stack_top = .); /* Define __stack_top for start.s */
}