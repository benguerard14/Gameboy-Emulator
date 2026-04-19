#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "cpu.h"
#include <stddef.h>
#include <stdint.h>

char *get_string_file(char *file, size_t *out_size);

uint8_t *pixels_from_tile(uint8_t tile[16]);

void emulator_init(char *rom, size_t size);

uint8_t *fetch_instruction();

typedef struct {
  uint8_t a;
} __attribute__((packed)) IORegs_t;

typedef struct {
  uint8_t rom_bank_0[0x3FFF];
  uint8_t rom_bank_1[0x3FFF];
  uint8_t VRAM[0x1FFF];
  uint8_t external_RAM[0x1FFF];
  uint8_t WRAM[0x1FFF];
  uint8_t echo_ram[0x1DFF];
  uint8_t OAM[0x9F];
  uint8_t no_access[0x5F];
  IORegs_t ioregs;
  uint8_t HRAM[0x7E];
  uint8_t interrupt_enable : 1;
} __attribute__((packed)) Memory_t;

typedef struct {
  Memory_t mem;
  CPU cpu;
} Gameboy;

#endif
