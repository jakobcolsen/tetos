#ifndef SBI_INIT_H
#define SBI_INIT_H

#include <fdt_parser.h>
#include <panic.h>

void sbi_init(const void* fdt_blob);

#endif // SBI_INIT_H