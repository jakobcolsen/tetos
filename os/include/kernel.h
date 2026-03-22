#ifndef KERNEL_H
#define KERNEL_H

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <uart.h>
#include <monitor.h>

void kprintf(const char* format_string, ...);
int kernel_main(void);

#endif // KERNEL_H