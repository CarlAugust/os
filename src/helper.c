#include "kernel.h"
#include <stdint.h>
#include <stdarg.h>

// Assumes null terminator
uint32_t strlen(const char* str) {
    uint32_t size = 0;
    while (str[size] != '\0') {
        size++;
    }
    return size;
}

void reverse(char* str) {
    uint32_t len = strlen(str);
    uint32_t l = 0;
    uint32_t r = len - 1;

    while (l < r) {
        char tmp = str[l];
        str[l] = str[r];
        str[r] = tmp; 
        l++;
        r--;
    }
}

void int_to_char(int v, char buffer[32]) {
    uint32_t n = 0;

    uint8_t negative = v < 0;
    if (negative) v *= (-1);

    do {
        buffer[n] = '0' + v % 10;
        v /= 10;
        n++;
    } while (v > 0);

    if (negative) buffer[n] = '-', n++;

    buffer[n] = '\0';
    reverse(buffer);
}

void print_int(int v) {
    char buffer[32];
    int_to_char(v, buffer);
    uart_puts(buffer);
}

/*
Assumes null terminator
Support only int and char* with %i and %s
*/
void printf(const char* str, ...) {

    va_list args;

    va_start(args, str);

    for (uint32_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == '%' && str[i + 1] != '\0') {
            if (str[i + 1] == 'i') {
                int n = va_arg(args, int);
                print_int(n);
                i++;
            } else if (str[i + 1] == 's') {
                char* s = va_arg(args, char*);
                uart_puts(s);
                i++;
            } else {

                uart_putc(str[i]);
            }
        } else {
            uart_putc(str[i]);
        }
    }

    va_end(args);
}