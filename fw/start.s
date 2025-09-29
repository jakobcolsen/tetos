.section .text
.globl _start

_start:
    la sp, __stack_top # Define stack
    mv a0, a1 # Move DTB to sbi_init
    call sbi_init

1:
    wfi
    j 1b