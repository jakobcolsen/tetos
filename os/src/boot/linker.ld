OUTPUT_ARCH(riscv)
ENTRY(_start)

/* QEMU virt: OpenSBI typically at 0x8000_0000; keep the S-mode kernel at 0x8020_0000 */
MEMORY
{
  RAM (rxw) : ORIGIN = 0x80200000, LENGTH = 128M
}

SECTIONS
{
  /* Helpful global bounds */
  __kernel_start = ORIGIN(RAM);
  . = __kernel_start;

  /* --- TEXT (S-mode entry + code) --- */
  .text ALIGN(0x1000) : ALIGN(0x1000)
  {
    /* Keep the entry stub first */
    KEEP(*(.text.start))
    *(.text .text.* .gnu.linkonce.t.*)
  } > RAM
  __text_end = .;

  /* --- RODATA --- */
  .rodata ALIGN(0x1000) : ALIGN(0x1000)
  {
    *(.rodata .rodata.* .gnu.linkonce.r.*)
  } > RAM
  __rodata_end = .;

  /* --- SMALL DATA WINDOW AROUND gp (optional but nice for -msmall-data) --- */
  /* Place .sdata/.sbss first, then define gp so both fit in ±2KB window */
  .sdata ALIGN(0x100) : ALIGN(0x100)
  {
    __sdata_start = .;
    *(.sdata .sdata.* .gnu.linkonce.s.*)
    __sdata_end = .;
  } > RAM

  .sbss ALIGN(0x100) : ALIGN(0x100)
  {
    __sbss_start = .;
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    __sbss_end = .;
  } > RAM

  /* The ELF symbol that start.S should load into gp */
  PROVIDE(__global_pointer$ = (__sdata_start + __sbss_end) / 2);

  /* --- DATA --- */
  .data ALIGN(0x1000) : ALIGN(0x1000)
  {
    __sdata = .;
    *(.data .data.* .gnu.linkonce.d.*)
    __edata = .;
  } > RAM

  /* --- BSS --- */
  .bss ALIGN(0x1000) (NOLOAD) : ALIGN(0x1000)
  {
    __bss_start = .;
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
    __bss_end = .;
  } > RAM

  /* End of kernel image */
  __kernel_end = .;

  /* Put the stack at the top of the RAM window we reserved for the kernel */
  __stack_top = ORIGIN(RAM) + LENGTH(RAM);

  /* Align for pages */
  . = ALIGN(0x1000);
}