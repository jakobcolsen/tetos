.section .text
.globl _start

_start:
    la sp, __stack_top // Define stack
    mv s0, a1 // Move DTB to static register

1:
    wfi
    j 1b