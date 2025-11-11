    .section .text.start
    .globl _start
    .type _start,@function
    .extern kernel_entry

_start:
    # Place SP just below the DTB (safe and inside RAM on QEMU virt)
    # I SPENT HOURS CHASING A RANDOM ASSORTMENT OF BUGS BECAUSE OF THIS ;-;
    mv   t2, a1          # a1 = dtb addr near top-of-RAM
    li   t1, -8192       # 8 KiB below DTB
    add  t2, t2, t1
    andi t2, t2, -16     # keep 16B alignment
    mv   sp, t2

    # Global pointer because gcc needs it
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop

    # Zero bss
    la      t0, __bss_start
    la      t1, __bss_end
1:  bgeu    t0, t1, 2f
    sd      zero, 0(t0)
    addi    t0, t0, 8
    j       1b
2:
    # jump to C preserving a0=hartid, a1=dtb
    tail    kernel_entry
