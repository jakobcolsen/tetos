#ifndef SBI_H
#define SBI_H
#include <stdint.h>

typedef struct { long error; long value; } sbi_ret_t;

// SBI ecall interface
static inline sbi_ret_t sbi_call(uint64_t eid, uint64_t fid,
                                 uint64_t a0, uint64_t a1, uint64_t a2,
                                 uint64_t a3, uint64_t a4, uint64_t a5)
{
    register uint64_t A0 asm("a0") = a0;
    register uint64_t A1 asm("a1") = a1;
    register uint64_t A2 asm("a2") = a2;
    register uint64_t A3 asm("a3") = a3;
    register uint64_t A4 asm("a4") = a4;
    register uint64_t A5 asm("a5") = a5;
    register uint64_t A6 asm("a6") = fid;
    register uint64_t A7 asm("a7") = eid;
    asm volatile("ecall"
                 : "+r"(A0), "+r"(A1)
                 : "r"(A2), "r"(A3), "r"(A4), "r"(A5), "r"(A6), "r"(A7)
                 : "memory");
    return (sbi_ret_t){ (long)A0, (long)A1 };
}

// Common EIDs/FIDs
enum {
    SBI_EID_BASE   = 0x10,        // Base extension
    SBI_EID_SRST   = 0x53525354,  // "SRST" system reset
    SBI_EID_TIMER  = 0x54494D45,  // "TIME" timer
    SBI_EID_IPI    = 0x735049,    // "sPI" IPI (platform dep.)
    SBI_EID_HSM    = 0x48534D,    // "HSM" hart state mgmt
};

enum { // Base
    SBI_FID_GET_SPEC_VERSION = 0,
    SBI_FID_GET_IMPL_ID      = 1,
    SBI_FID_GET_IMPL_VERSION = 2,
};

enum { // SRST
    SBI_SRST_TYPE_SHUTDOWN = 0,
    SBI_SRST_TYPE_COLD     = 1,
    SBI_SRST_TYPE_WARM     = 2,
    SBI_SRST_REASON_NONE   = 0,
};

static inline void sbi_system_shutdown(void) {
    (void)sbi_call(SBI_EID_SRST, 0, SBI_SRST_TYPE_SHUTDOWN, SBI_SRST_REASON_NONE, 0,0,0,0);
}

static inline void sbi_system_reboot_cold(void) {
    (void)sbi_call(SBI_EID_SRST, 0, SBI_SRST_TYPE_COLD, SBI_SRST_REASON_NONE, 0,0,0,0);
}

static inline void sbi_console_putc(int ch) {
  register uint64_t A0 asm("a0") = (uint8_t)ch;
  register uint64_t A7 asm("a7") = 1;  // legacy putc
  asm volatile("ecall" : "+r"(A0) : "r"(A7) : "memory");
}

static inline void sbi_set_timer(uint64_t stime) {  // EID "TIME", FID set_timer=0
  register uint64_t A0 asm("a0") = stime;
  register uint64_t A6 asm("a6") = 0;
  register uint64_t A7 asm("a7") = SBI_EID_TIMER;
  asm volatile("ecall" : "+r"(A0) : "r"(A6), "r"(A7) : "memory");
}

static inline uint64_t rdtime(void) { 
    uint64_t x; asm volatile("csrr %0, time":"=r"(x)); 
    return x; 
}

#endif // SBI_H