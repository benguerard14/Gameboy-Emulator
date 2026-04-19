#ifndef EMULATOR_H
#define EMULATOR_H

#include <stdint.h>

char *get_string_file(char *file);

uint8_t* pixels_from_tile(uint8_t tile[16]);

typedef struct {
  uint8_t a;
} __attribute__((packed)) IORegs_t;


typedef union {
    struct {
        uint8_t lo;
        uint8_t hi;
    };
    uint16_t val;
} reg16;

typedef union {
    struct {
        uint8_t F;  // flags (low byte)
        uint8_t A;  // accumulator (high byte)
    };
    struct {
        uint8_t unused:4;
        uint8_t C:1;
        uint8_t H:1;
        uint8_t N:1;
        uint8_t Z:1;
        uint8_t A_pad; // aligns with A
    } flags;
    uint16_t AF;
} regAF;

typedef struct {
  reg16 AF;
  reg16 BF;
  reg16 DE;
  reg16 HL;
  reg16 SP;
  regAF PC;
}__attribute__((packed)) CPU;

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

typedef struct{
  Memory_t mem;
  CPU cpu;
} Gameboy;

#endif
