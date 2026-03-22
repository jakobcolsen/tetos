#ifndef RISCV_CSR_H
#define RISCV_CSR_H

#include <stdint.h>

/* S-mode interrupt-enable bits (SIE register) */
#define SIE_SSIE  (1ULL << 1)   /* Supervisor software interrupt */
#define SIE_STIE  (1ULL << 5)   /* Supervisor timer interrupt   */
#define SIE_SEIE  (1ULL << 9)   /* Supervisor external interrupt */

/* SSTATUS bits */
#define SSTATUS_SIE  (1ULL << 1) /* Supervisor interrupt enable */

/* SCAUSE helpers */
#define SCAUSE_INT_MASK  (1ULL << 63)
#define SCAUSE_CODE(x)   ((x) & 0xFFFULL)

#endif // RISCV_CSR_H
