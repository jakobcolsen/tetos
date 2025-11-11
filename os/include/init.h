#ifndef INIT_H
#define INIT_H

#include <fdt_parser.h>
#include <panic.h>
#include <uart.h>
#include <timer.h>

#define UART_DEFAULT_MAP 0x10000000ull

void init(const void* fdt_blob);

#endif // INIT_H