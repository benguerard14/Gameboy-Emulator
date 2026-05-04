#include "gameboy.h"
#include "mmu.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *get_string_file(char *file, size_t *out_size) {
  FILE *f = fopen(file, "rb");
  if (!f) {
    printf("ERROR: Failed to Open ROM\n");
    exit(-1);
  }

  fseek(f, 0, SEEK_END);
  size_t file_size = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *buff = malloc(file_size);
  if (!buff) {
    printf("ERROR: Failed to allocate memory\n");
    fclose(f);
    exit(-1);
  }

  size_t n = fread(buff, 1, file_size, f);
  fclose(f);

  if (n != file_size) {
    printf("ERROR: Failed to read complete file\n");
    free(buff);
    exit(-1);
  }

  if (out_size) {
    *out_size = file_size;
  }

  return buff;
}

void emulator_init(Gameboy *gb, char *rom, size_t size) {
  // mmcpy to be changed
  memcpy(gb->mem.rom_bank_0, rom, 0x4000);
  memcpy(gb->mem.rom_bank_1, rom + 0x4000, 0x4000);
  gb->cpu.PC.val = 0x0100;
  gb->cpu.SP.val = 0xFFFE;
  gb->cpu.AF.AF = 0x01B0;
  gb->cpu.BC.val = 0x0013;
  gb->cpu.DE.val = 0x00D8;
  gb->cpu.HL.val = 0x014D;
  gb->cpu.IME = 0;

  mem_write(&gb->mem, 0xFF05, 0x00); // TIMA
  mem_write(&gb->mem, 0xFF06, 0x00); // TMA
  mem_write(&gb->mem, 0xFF07, 0x00); // TAC
  mem_write(&gb->mem, 0xFF10, 0x80); // NR10
  mem_write(&gb->mem, 0xFF11, 0xBF); // NR11
  mem_write(&gb->mem, 0xFF12, 0xF3); // NR12
  mem_write(&gb->mem, 0xFF14, 0xBF); // NR14
  mem_write(&gb->mem, 0xFF16, 0x3F); // NR21
  mem_write(&gb->mem, 0xFF17, 0x00); // NR22
  mem_write(&gb->mem, 0xFF19, 0xBF); // NR24
  mem_write(&gb->mem, 0xFF1A, 0x7F); // NR30
  mem_write(&gb->mem, 0xFF1B, 0xFF); // NR31
  mem_write(&gb->mem, 0xFF1C, 0x9F); // NR32
  mem_write(&gb->mem, 0xFF1E, 0xBF); // NR33
  mem_write(&gb->mem, 0xFF20, 0xFF); // NR41
  mem_write(&gb->mem, 0xFF21, 0x00); // NR42
  mem_write(&gb->mem, 0xFF22, 0x00); // NR43
  mem_write(&gb->mem, 0xFF23, 0xBF); // NR44
  mem_write(&gb->mem, 0xFF24, 0x77); // NR50
  mem_write(&gb->mem, 0xFF25, 0xF3); // NR51
  mem_write(&gb->mem, 0xFF26, 0xF1); // NR52
  mem_write(&gb->mem, 0xFF40, 0x91); // LCDC
  mem_write(&gb->mem, 0xFF42, 0x00); // SCY
  mem_write(&gb->mem, 0xFF43, 0x00); // SCX
  mem_write(&gb->mem, 0xFF45, 0x00); // LYC
  mem_write(&gb->mem, 0xFF47, 0xFC); // BGP
  mem_write(&gb->mem, 0xFF48, 0xFF); // OBP0
  mem_write(&gb->mem, 0xFF49, 0xFF); // OBP1
  mem_write(&gb->mem, 0xFF4A, 0x00); // WY
  mem_write(&gb->mem, 0xFF4B, 0x00); // WX
  mem_write(&gb->mem, 0xFFFF, 0x00); // IE
}
