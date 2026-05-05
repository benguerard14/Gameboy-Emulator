#include "mmu.h"
#include <stdio.h>

void handle_serial_transfer(uint8_t c) {
  // putchar(c);
  fflush(stdout);
}

void mem_write(Memory_t *mem, uint16_t addr, uint8_t val) {
  if (addr < 0x2000) {
    mem->ram_enabled = (val & 0x0F) == 0x0A;
    return;
  }
  if (addr < 0x4000) {
    uint8_t bank = val & 0x1F;
    if (bank == 0)
      bank = 1;
    mem->rom_bank = (mem->rom_bank & 0x60) | bank;
    return;
  }
  if (addr < 0x6000) {
    if (mem->mbc1_mode == 0)
      mem->rom_bank =
          (mem->rom_bank & 0x1F) | ((val & 0x03) << 5); // add 2 bits on top
    else
      mem->ram_bank = val & 0x03; // or select RAM bank
    return;
  }
  if (addr < 0x8000) {
    mem->mbc1_mode = val & 0x01; // 0 or 1
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
    return mem->full_rom[mem->rom_bank * 0x4000 + (addr - 0x4000)];
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
  if (addr == 0xFF00) {
    uint8_t select = mem->ioregs[0];
    uint8_t result = 0xFF;

    if (!(select & 0x20)) {
      // action buttons: A, B, Select, Start (bits 0-3)
      result &= (mem->joypad_action | 0xF0);
    }
    if (!(select & 0x10)) {
      // directional: Right, Left, Up, Down (bits 0-3)
      result &= (mem->joypad_direction | 0xF0);
    }
    return result;
  }

  if (addr < 0xFF80)
    return mem->ioregs[addr - 0xFF00];

  if (addr < 0xFFFF)
    return mem->HRAM[addr - 0xFF80];

  return mem->interrupt_enable;
}

void set_joypad_action(Memory_t *mem, uint8_t bit, uint8_t pressed) {
  if (pressed)
    mem->joypad_action &= ~(1 << bit); // 0 = pressed
  else
    mem->joypad_action |= (1 << bit); // 1 = released
}

void set_joypad_dir(Memory_t *mem, uint8_t bit, uint8_t pressed) {
  if (pressed)
    mem->joypad_direction &= ~(1 << bit);
  else
    mem->joypad_direction |= (1 << bit);
}
