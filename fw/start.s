.section .text
.globl _start

_start:
    la sp, __stack_top # Define stack
    mv a0, a1 # Move DTB to static register
    call sbi_init;

1:
    wfi
    j 1b