#include <kernel/kernel.h>
#include <stdint.h>
#include <stdarg.h>


void memset(void* ptr, int c, uint32_t n) {
    uint8_t *p = (uint8_t*)(ptr);
    while(n--) {
        *p = (uint8_t)c;
        ptr++;
    } 
}

// Assumes null terminator
uint32_t strlen(const char* str) {
    uint32_t size = 0;
    while (str[size] != '\0') size++;

    return size;
}


