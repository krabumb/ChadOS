#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

#define SERIAL_PORT 0x3F8  // Port address for COM1

void serial_init();
void serial_write_char(char c);
void serial_write(const char* s);
void print_int(uint32_t x);
void print_hex(uint32_t x);

#endif // SERIAL_H
