#include <stdint.h>
#include <stddef.h>

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

void* memset(void* destination, int value, size_t n) {
    unsigned char* d = (unsigned char*) destination;
    unsigned char v = (unsigned char) value;
    for (size_t i = 0; i < n; i++) {
        d[i] = v;
    }

    return destination;
}

void memmove(void* destination, const void* source, size_t n) {
    unsigned char* dest = (unsigned char*) destination;
    const unsigned char* src = (const unsigned char*) source;

    if (dest == src || n == 0) return destination;

    if (dest < src) {
        for (size_t i = 0; i < n; i++) {
            dest[i] = src[i];
        }
    } else if (dest > src) {
        for (size_t i = n; i != 0; i--) {
            dest[i - 1] = src[i - 1];
        }
    }

    return destination;
}

