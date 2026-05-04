#include "mmu.h"
#include <stdio.h>

void handle_serial_transfer(uint8_t c) {
  putchar(c);
  fflush(stdout);
}

void mem_write(Memory_t *mem, uint16_t addr, uint8_t val) {
  if (addr < 0x8000) {
    // ROM → usually ignore or handle MBC later
    return;
  }
  if (addr < 0xA000) {
    mem->VRAM[addr - 0x8000] = val;
    return;
  }
  if (addr < 0xC000) {
    mem->external_RAM[addr - 0xA000] = val;
    return;
  }
  if (addr < 0xE000) {
    mem->WRAM[addr - 0xC000] = val;
    return;
  }

  // Echo RAM → write to WRAM
  if (addr < 0xFE00) {
    mem->WRAM[addr - 0xE000] = val;
    return;
  }

  if (addr < 0xFEA0) {
    mem->OAM[addr - 0xFE00] = val;
    return;
  }

  if (addr < 0xFF00) {
    return; // unusable
  }

  if (addr < 0xFF80) {
    mem->ioregs[addr - IO_REGS_ADDR] = val;
    // Check if serial data transfer
    if (addr == SERIAL_DATA_CTRL) {
      if (val & 0x80) {
        handle_serial_transfer(mem->ioregs[SERIAL_DATA_OUT - IO_REGS_ADDR]);
        mem->ioregs[SERIAL_DATA_CTRL - IO_REGS_ADDR] &= 0x7F;
      }
    } else if (addr == TIMER_DIV_ADDR) {
      mem->ioregs[addr - IO_REGS_ADDR] = 0;
    } else if (addr == 0xFF46) {
      uint16_t source = val << 8;

      for (int i = 0; i < 0xA0; i++) {
        uint8_t byte = mem_read(mem, source + i);
        mem->OAM[i] = byte;
      }
    }

    return;
  }

  if (addr < 0xFFFF) {
    mem->HRAM[addr - 0xFF80] = val;
    return;
  }

  mem->interrupt_enable = val;
}

uint8_t mem_read(Memory_t *mem, uint16_t addr) {
  if (addr < 0x4000)
    return mem->rom_bank_0[addr];
  if (addr < 0x8000)
    return mem->rom_bank_1[addr - 0x4000];
  if (addr < 0xA000)
    return mem->VRAM[addr - 0x8000];
  if (addr < 0xC000)
    return mem->external_RAM[addr - 0xA000];
  if (addr < 0xE000)
    return mem->WRAM[addr - 0xC000];

  // Echo RAM (mirror WRAM)
  if (addr < 0xFE00)
    return mem->WRAM[addr - 0xE000];

  if (addr < 0xFEA0)
    return mem->OAM[addr - 0xFE00];

  if (addr < 0xFF00)
    return 0xFF;

  if (addr < 0xFF80)
    return mem->ioregs[addr - 0xFF00];

  if (addr < 0xFFFF)
    return mem->HRAM[addr - 0xFF80];

  return mem->interrupt_enable;
}
