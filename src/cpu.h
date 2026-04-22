#ifndef CPU_H
#define CPU_H

#include "gameboy.h"
#include <stdint.h>

typedef union {
  struct {
    uint8_t lo;
    uint8_t hi;
  };
  uint16_t val;
} reg16;

typedef union {
  struct {
    uint8_t F; // flags (low byte)
    uint8_t A; // accumulator (high byte)
  };
  struct {
    uint8_t unused : 4;
    uint8_t C : 1;
    uint8_t H : 1;
    uint8_t N : 1;
    uint8_t Z : 1;
    uint8_t A_pad; // aligns with A
  } flags;
  uint16_t AF;
} regAF;

typedef struct {
  regAF AF;
  reg16 BC;
  reg16 DE;
  reg16 HL;
  reg16 SP;
  reg16 PC;
} CPU;

uint8_t cpu_step(uint8_t *ins, CPU *cpu, Memory_t *mem);

#endif
