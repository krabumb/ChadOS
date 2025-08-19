#include "serial.h"
#include <stdint.h>

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void serial_init() {
    outb(SERIAL_PORT + 1, 0x00);
    outb(SERIAL_PORT + 3, 0x80);
    outb(SERIAL_PORT + 0, 0x03);
    outb(SERIAL_PORT + 1, 0x00);
    outb(SERIAL_PORT + 3, 0x03);
    outb(SERIAL_PORT + 2, 0xC7);
    outb(SERIAL_PORT + 4, 0x0B);
}

void serial_write_char(char c) {
    while ((inb(SERIAL_PORT + 5) & 0x20) == 0);
    outb(SERIAL_PORT, c);
}

void serial_write(const char* s) {
    // null check
    if (!s) return;

    for (; *s; ++s) {
        if (*s == '\n') serial_write_char('\r');
        serial_write_char(*s);
    }
}

void print_int(uint32_t x) {
    char buf[11];
    int i = 10;
    buf[i--] = '\0';
    do {
        buf[i--] = '0' + (x % 10);
        x /= 10;
    } while (x && i >= 0);
    serial_write(&buf[i + 1]);
}

void print_hex(uint32_t x) {
    char* hex = "0123456789ABCDEF";
    serial_write("0x");
    for (int i = 7; i >= 0; --i) {
        char c = hex[(x >> (i * 4)) & 0xF];
        char s[2] = { c, 0 };
        serial_write(s);
    }
}

