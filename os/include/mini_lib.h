#include <stdint.h>
#include <stddef.h>

#ifndef MINI_LIB_H
#define MINI_LIB_H

void* memcpy(void* dest, const void* src, size_t n);
int strcmp(const char* s1, const char* s2);
int strlen(const char* s);
int memcmp(const void* a, const void* b, size_t n);
void* memset(void* destination, int value, size_t n);
void* memmove(void* destination, const void* source, size_t n);

#endif // MINI_LIB_H