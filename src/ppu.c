#include "ppu.h"
#include "cpu.h"
#include "mmu.h"

void ppu_step(Memory_t *mem, PPU *ppu, uint8_t cycles) {
  ppu->dots += cycles * 4;
  while (ppu->dots >= 456) {
    ppu->line = (ppu->line + 1) % 154;
    ppu->dots -= 456;
    ppu->mode3_extra = 0;

    mem_write(mem, LCD_Y_COORD, ppu->line);

    if (ppu->line == 144) {
      ppu->mode = 1;
      request_interrupt(mem, 0);
    }

    else if (ppu->line < 144) {
      ppu->mode = 2;
    }
  }

  if (ppu->mode == 1) {
    return;
  }

  if (ppu->mode == 2 && ppu->dots >= 80) {
    ppu->mode = 3;
  }
  if (ppu->mode == 3 && ppu->dots >= 80 + 172 + ppu->mode3_extra) {
    ppu->mode = 0;
  }
}
