#ifndef UTIL_H
#define UTIL_H

#include "gameboy.h"

void mem_write(Memory_t *mem, uint16_t addr, uint8_t val);

uint8_t mem_read(Memory_t *mem, uint16_t addr);

#endif
