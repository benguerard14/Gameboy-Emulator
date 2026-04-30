#ifndef PPU_H
#define PPU_H

#include "cpu.h"
#include "mmu.h"

typedef struct {
  uint16_t dots;
  uint16_t accumulated_dots;
  uint16_t mode;
  uint16_t lines;
  uint16_t mode3_extra;
  uint8_t stat_interrupted;
} PPU;

void ppu_step(Memory_t *mem, PPU *ppu, uint8_t cycles);

#endif
