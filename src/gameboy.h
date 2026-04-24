#ifndef GAMEBOY_H
#define GAMEBOY_H

#include <stddef.h>
#include <stdint.h>

char *get_string_file(char *file, size_t *out_size);

uint8_t *pixels_from_tile(uint8_t tile[16]);

void emulator_init(char *rom, size_t size);

uint8_t fetch_instruction();

typedef struct {
  uint8_t a;
} __attribute__((packed)) IORegs_t;

typedef struct {
  uint8_t rom_bank_0[0x4000];
  uint8_t rom_bank_1[0x4000];
  uint8_t VRAM[0x2000];
  uint8_t external_RAM[0x2000];
  uint8_t WRAM[0x2000];
  uint8_t echo_ram[0x1E00];
  uint8_t OAM[0xA0];
  uint8_t no_access[0x60];
  IORegs_t ioregs;
  uint8_t HRAM[0x7F];
  uint8_t interrupt_enable;
} __attribute__((packed)) Memory_t;

#endif
