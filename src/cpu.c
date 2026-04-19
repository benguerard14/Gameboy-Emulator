#include "cpu.h"
#include "gameboy.h"
#include <stdio.h>
#include <stdlib.h>

void load_r16(uint16_t val, uint16_t *dest) { *dest = val; }

uint8_t cpu_step(uint8_t *ins, CPU *cpu) {
  // nop -> do nothing
  if ((*ins & 0xFF) == 0) {
    return 1;
  }
  uint8_t block0_mask = 0b11001111;
  // ld r16, imm16
  if ((*ins & block0_mask) == 1) {
    uint16_t val = (*fetch_instruction() << 8) + *fetch_instruction();
    reg16 *reg = &cpu->BC + ((~block0_mask & *ins) >> 4);
    load_r16(val, &reg->val);
    return 3;
  }

  printf("ERROR: Invalid Instruction: %02X\n", *ins);
  exit(-1);
  return 0;
}
