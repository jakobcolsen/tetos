#include <kprintf.h>

static void print_value(int value, int base, int is_signed) {
    char buffer[32]; // Enough for 32-bit binary representation + sign + null
    char* p = &buffer[31]; // Reverse fill from the end
    *p = '\0';

    int is_negative = 0;
    unsigned int uvalue;

    // Handle negative numbers for signed decimal
    if (is_signed && value < 0) {
        is_negative = 1;
        uvalue = (unsigned int) (-value);
    } else {
        uvalue = (unsigned int) value;
    }

    // Convert number to string in reverse order
    if (uvalue == 0) {
        *--p = '0';
    } else {
        while (uvalue > 0) {
            int digit = uvalue % base;
            *--p = (digit < 10) ? ('0' + digit) : ('a' + (digit - 10)); // Decode hex digits
            uvalue /= base; // Unsigned division
        }
    }

    // Add negative sign if needed
    if (is_negative) {
        *--p = '-';
    }

    uart_puts(p);
}

void kprintf(const char* format_string, ...) {
    // va_list handles variable arguments
    // va_start initializes it
    va_list args;
    va_start(args, format_string);

    const char* p = format_string;
    while (*p) {
        // Need not apply
        if (*p != '%') {
            uart_putc(*p++);
            continue;
        }

        p++; // Skip '%'
        switch (*p) {
            case 'c': {
                // va_arg reads the next argument of the given type
                char c = (char) va_arg(args, int); // char promoted to int, congratulations char o7
                uart_putc(c);
                break;
            } // char
            case 's': {
                const char* str = va_arg(args, const char*);
                uart_puts(str);
                break;
            } // char*
            case 'i': // Also decimal because why not
            case 'd':{
                int value = va_arg(args, int);
                print_value(value, 10, 1);
                break;
            } // decimal
            case 'u': {
                unsigned int uvalue = va_arg(args, unsigned int);
                print_value(uvalue, 10, 0);
                break;
            } // unsigned decimal
            case 'x': {
                unsigned int hex = va_arg(args, unsigned int);
                print_value(hex, 16, 0);
                break;
            } // hex
            case '%': {
                uart_putc('%');
                break;
            } // It's just a percent sign
            default: {
                uart_putc('%');
                uart_putc(*p);
                break;
            } // Not s,i,d,u,x,c,%
        }
        p++; // Move past format specifier
    }

    // clean up
    va_end(args);
}