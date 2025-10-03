#ifndef KPRINTF_H
#define KPRINTF_H

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <uart.h>

void kprintf(const char* format_string, ...);

#endif // KPRINTF_H