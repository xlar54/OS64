#ifndef __STDIO_H
#define __STDIO_H

#include <lib/stdint.h>
#include <lib/stdarg.h>
#include <lib/vga.h>

void printf(char* str,...);

void displayMemory(uint8_t* buffer, uint16_t size);

#endif