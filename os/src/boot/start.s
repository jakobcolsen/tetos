.section .text.start
.globl _start

_start:
    la sp, __stack_top
    mv a0, a1;

1:
    call kernel_entry      /* C entry */
hang:
    wfi
    j       hang
