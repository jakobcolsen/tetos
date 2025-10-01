#ifndef PANIC_H
#define PANIC_H

#include <kprintf.h>

#define panic(msg) _panic(msg, __FILE__, __LINE__, __func__)

void _panic(const char* msg, const char* file, int line, const char* func);

#endif // PANIC_H