#include <stdint.h>

void* memcpy(void* dest, const void* src, size_t n) {
    char *d = (char*) dest;
    const char *s = (const char*) src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dest;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }

    return *(const unsigned char*) s1 - *(const unsigned char*) s2;
}

size_t strlen(const char* s) {
    size_t n = 0;
    while (s[n]) n++;

    return n;
}

int memcmp(const void* a, const void* b, size_t n) {
    const unsigned char* x = (const unsigned char*) a;
    const unsigned char* y = (const unsigned char*) b;
    for (size_t i = 0; i < n; i++) {
        if (x[i] != y[i]) return (int) x[i] - (int) y[i];
    }

    return 0;
}