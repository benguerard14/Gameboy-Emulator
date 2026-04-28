#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

typedef struct {
  uint8_t rom_bank_0[0x4000];
  uint8_t rom_bank_1[0x4000];
  uint8_t VRAM[0x2000];
  uint8_t external_RAM[0x2000];
  uint8_t WRAM[0x2000];
  uint8_t echo_ram[0x1E00];
  uint8_t OAM[0xA0];
  uint8_t no_access[0x60];
  uint8_t ioregs[0x80];
  uint8_t HRAM[0x7F];
  uint8_t interrupt_enable;
} __attribute__((packed)) Memory_t;

void mem_write(Memory_t *mem, uint16_t addr, uint8_t val);

uint8_t mem_read(Memory_t *mem, uint16_t addr);

#endif
