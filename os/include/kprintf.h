#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <uart.h>

#ifndef KPRINTF_H
#define KPRINTF_H

void kprintf(const char* format_string, ...);

#endif // KPRINTF_H