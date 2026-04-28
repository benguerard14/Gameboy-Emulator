#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "cpu.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
  Memory_t mem;
  CPU cpu;
} Gameboy;

char *get_string_file(char *file, size_t *out_size);

uint8_t *pixels_from_tile(uint8_t tile[16]);

void emulator_init(Gameboy *gb, char *rom, size_t size);

#endif
