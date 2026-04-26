#include "cpu.h"
#include "gameboy.h"
#include "util.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t get_imm8() { return fetch_instruction(); }

uint16_t get_imm16() {
  return (fetch_instruction()) + (fetch_instruction() << 8);
}

void set_flags(CPU *cpu, uint8_t Z, uint8_t N, uint8_t H, uint8_t C) {
  cpu->AF.flags.C = C;
  cpu->AF.flags.H = H;
  cpu->AF.flags.N = N;
  cpu->AF.flags.Z = Z;
}

uint8_t get_condition(uint8_t condition_num, CPU *cpu) {
  if (condition_num == 0x0) {
    return (!cpu->AF.flags.Z); // NZ
  }
  if (condition_num == 0x1) {
    return (cpu->AF.flags.Z); // Z
  }
  if (condition_num == 0x2) {
    return (!cpu->AF.flags.C); // NC
  }
  if (condition_num == 0x3) {
    return (cpu->AF.flags.C); // C
  }
  printf("ERROR: Invalid Condition Number\n");
  exit(-1);
}

uint8_t get_reg8(uint8_t reg_num, CPU *cpu, Memory_t *mem) {
  switch (reg_num) {
  case 0x0:
    return cpu->BC.hi;
  case 0x1:
    return cpu->BC.lo;
  case 0x2:
    return cpu->DE.hi;
  case 0x3:
    return cpu->DE.lo;
  case 0x4:
    return cpu->HL.hi;
  case 0x5:
    return cpu->HL.lo;
  case 0x6:
    return mem_read(mem, cpu->HL.val);
  case 0x7:
    // GET A
    return cpu->AF.A;
  default:
    printf("ERROR: Getting invalid reg8\n");
    exit(-1);
  }
}

void set_reg8(uint8_t reg_num, CPU *cpu, Memory_t *mem, uint8_t val) {
  switch (reg_num) {
  case 0x0:
    cpu->BC.hi = val;
    break;
  case 0x1:
    cpu->BC.lo = val;
    break;
  case 0x2:
    cpu->DE.hi = val;
    break;
  case 0x3:
    cpu->DE.lo = val;
    break;
  case 0x4:
    cpu->HL.hi = val;
    break;
  case 0x5:
    cpu->HL.lo = val;
    break;
  case 0x6:
    mem_write(mem, cpu->HL.val, val);
    break;
  case 0x7:
    // GET A
    cpu->AF.A = val;
    break;
  default:
    printf("ERROR: Setting invalid reg8\n");
    exit(-1);
  }
}

uint16_t get_reg16(uint8_t reg_num, CPU *cpu) {
  switch (reg_num) {
  case 0x0:
    return cpu->BC.val;
  case 0x1:
    return cpu->DE.val;
  case 0x2:
    return cpu->HL.val;
  case 0x3:
    return cpu->SP.val;
  default:
    printf("ERROR: Getting invalid reg16\n");
    exit(-1);
  }
}

void set_reg16(uint8_t reg_num, CPU *cpu, uint16_t val) {
  switch (reg_num) {
  case (0x0):
    cpu->BC.val = val;
    return;
  case (0x1):
    cpu->DE.val = val;
    return;
  case (0x2):
    cpu->HL.val = val;
    return;
  case (0x3):
    cpu->SP.val = val;
    return;
  default:
    printf("ERROR: Setting invalid reg16\n");
    exit(-1);
  }
}

uint16_t get_memreg16(uint8_t reg_num, CPU *cpu) {
  switch (reg_num) {
  case 0x0:
    return cpu->BC.val;
  case 0x1:
    return cpu->DE.val;

  case 0x2:
    cpu->HL.val++;
    return (cpu->HL.val - 1);
  case 0x3:
    cpu->HL.val--;
    return (cpu->HL.val + 1);

  default:
    printf("ERROR: Invalid memreg16\n");
    exit(-1);
  }
}

uint8_t cpu_step(uint8_t ins, CPU *cpu, Memory_t *mem) {
  // BLOCK 0 INSTRUCTIONS
  //  nop -> do nothing
  if ((ins & 0xFF) == 0) {
    return 1;
  }
  uint8_t block0_16mask = 0b11001111;
  uint8_t block0_8mask = 0b11000111;
  uint8_t block0_16sel = (ins >> 4) & 0b11;
  uint8_t block0_8sel = (ins >> 3) & 0b111;
  uint8_t condition_mask = 0b11100111;
  // ld r16, imm16
  if ((ins & block0_16mask) == 0x1) {
    uint16_t val = get_imm16();
    set_reg16(block0_16sel, cpu, val);
    return 3;
  }
  // LD [r16_mem],A
  if ((ins & block0_16mask) == 0x2) {
    uint16_t addr = get_memreg16(block0_16sel, cpu);
    mem_write(mem, addr, cpu->AF.A);
    return 2;
  }
  // LD A,[r16_mem]
  if ((ins & block0_16mask) == 0xA) {
    uint16_t addr = get_memreg16(block0_16sel, cpu);
    uint8_t val = mem_read(mem, addr);
    cpu->AF.A = val;
    return 2;
  }
  // ld [imm16], sp
  if (ins == 0x8) {
    uint16_t addr = get_imm16();
    mem_write(mem, addr, cpu->SP.lo);
    mem_write(mem, addr + 1, cpu->SP.hi);
    return 5;
  }

  // inc r16
  if ((ins & block0_16mask) == 0x3) {
    uint16_t val = get_reg16(block0_16sel, cpu);
    set_reg16(block0_16sel, cpu, val + 1);
    return 2;
  }
  // dec r16
  if ((ins & block0_16mask) == 0xB) {
    uint16_t val = get_reg16(block0_16sel, cpu);
    set_reg16(block0_16sel, cpu, val - 1);
    return 2;
  }
  // add hl, r16
  if ((ins & block0_16mask) == 0x9) {
    uint16_t val = get_reg16(block0_16sel, cpu);
    uint32_t sum = val + cpu->HL.val;
    set_flags(cpu, cpu->AF.flags.Z, 0,
              ((cpu->HL.val & 0x0FFF) + (val & 0x0FFF)) > 0x0FFF,
              (sum > 0xFFFF));
    cpu->HL.val = (uint16_t)sum;
    return 2;
  }

  // inc r8
  if ((ins & block0_8mask) == 0x4) {
    uint8_t val = get_reg8(block0_8sel, cpu, mem);
    uint8_t result = val + 1;
    set_reg8(block0_8sel, cpu, mem, result);
    set_flags(cpu, (result) == 0, 0, (val & 0x0F) == 0x0F, cpu->AF.flags.C);
    return 1;
  }
  // dec r8
  if ((ins & block0_8mask) == 0x5) {
    uint8_t val = get_reg8(block0_8sel, cpu, mem);
    uint8_t result = val - 1;
    set_reg8(block0_8sel, cpu, mem, result);
    set_flags(cpu, (result) == 0, 1, (val & 0x0F) == 0x0, cpu->AF.flags.C);
    return 1;
  }

  // ld r8, n8
  if ((ins & block0_8mask) == 0x6) {
    uint8_t val = get_imm8();
    set_reg8(block0_8sel, cpu, mem, val);
    return 2;
  }

  // rlca: rotate register A left
  if (ins == 0x7) {
    uint8_t val = cpu->AF.A;
    uint8_t carry = (val >> 7) & 0b1;
    uint8_t result = (val << 1) | carry;
    set_flags(cpu, 0, 0, 0, carry);
    cpu->AF.A = result;
    return 1;
  }
  // rrca: rotate register A right
  if (ins == 0xF) {
    uint8_t val = cpu->AF.A;
    uint8_t carry = val & 0b1;
    uint8_t result = (val >> 1) | (carry << 7);
    set_flags(cpu, 0, 0, 0, carry);
    cpu->AF.A = result;
    return 1;
  }
  // rla: rotate register A left through the carry flag
  if (ins == 0x17) {
    uint8_t val = cpu->AF.A;
    uint8_t result = (val << 1) | cpu->AF.flags.C;
    uint8_t carry = (val >> 7) & 0b1;
    set_flags(cpu, 0, 0, 0, carry);
    cpu->AF.A = result;
    return 1;
  }
  // rra: rotate register A right through the carry flag
  if (ins == 0x1F) {
    uint8_t val = cpu->AF.A;
    uint8_t result = (val >> 1) | (cpu->AF.flags.C << 7);
    uint8_t carry = val & 0b1;
    set_flags(cpu, 0, 0, 0, carry);
    cpu->AF.A = result;
    return 1;
  }
  // daa: decimal adjust
  /* If the subtract flag N is set:

     Initialize the adjustment to 0.
     If the half-carry flag H is set, then add $6 to the adjustment.
     If the carry flag is set, then add $60 to the adjustment.
     Subtract the adjustment from A.


     If the subtract flag N is not set:

     Initialize the adjustment to 0.
     If the half-carry flag H is set or A & $F > $9, then add $6 to the
     adjustment. If the carry flag is set or A > $99, then add $60 to the
     adjustment and set the carry flag. Add the adjustment to A.
     */
  if (ins == 0x27) {
    uint8_t adj = 0;
    uint8_t carry = cpu->AF.flags.C;

    if (cpu->AF.flags.N) {
      if (cpu->AF.flags.H) {
        adj += 0x6;
      }
      if (cpu->AF.flags.C) {
        adj += 0x60;
      }
      cpu->AF.A -= adj;
    } else {
      if (cpu->AF.flags.H || (cpu->AF.A & 0xF) > 0x9) {
        adj += 0x6;
      }
      if (cpu->AF.flags.C || (cpu->AF.A) > 0x99) {
        adj += 0x60;
        carry = 1;
      }
      cpu->AF.A += adj;
    }

    set_flags(cpu, (cpu->AF.A == 0), cpu->AF.flags.N, 0, carry);
    return 1;
  }
  // cpl: A = ~A
  if (ins == 0x2F) {
    cpu->AF.A = ~(cpu->AF.A);
    set_flags(cpu, cpu->AF.flags.Z, 1, 1, cpu->AF.flags.C);
    return 1;
  }
  // scf: set carry flag
  if (ins == 0x37) {
    set_flags(cpu, cpu->AF.flags.Z, 0, 0, 1);
    return 1;
  }
  // ccf: complement carry flag (c = ~c)
  if (ins == 0x3F) {
    set_flags(cpu, cpu->AF.flags.Z, 0, 0, !cpu->AF.flags.C);
    return 1;
  }

  // jr imm8: jump by offsetting with imm8
  if (ins == 0x18) {
    cpu->PC.val += (int8_t)get_imm8();
    return 3;
  }

  // jr cc, imm8: jump by offsetting if cc
  if ((ins & condition_mask) == 0x20) {
    if (get_condition((ins >> 3) & 0x3, cpu)) {
      cpu->PC.val += (int8_t)get_imm8();
      return 3;
    }
    return 2;
  }

  // stop: no idea but looks even worse than halt
  if (ins == 0x10) {
    get_imm8();

    printf("Stop instruction. No idea what to do\n");

    return 2;
  }

  // BLOCK 1
  uint8_t reg_mask = 0b11000000;
  //  halt: absolutely no idea what it does and hope i don't have to find out
  if (ins == 0x76) {
    printf("Halt instruction. No idea what to do\n");
    return 2;
  }
  // ld r8, r8
  if ((ins & reg_mask) == 0x40) {
    uint8_t val = get_reg8(ins & 0b111, cpu, mem);
    set_reg8((ins >> 3) & 0b111, cpu, mem, val);
    return 1;
  }

  // BLOCK 2
  uint8_t arithmetic_block = 0b11111000;
  // add a, r8: add the value of r8 to a
  if ((ins & arithmetic_block) == 0x80) {
    uint8_t a = cpu->AF.A;
    uint8_t r8 = get_reg8(ins & 0b111, cpu, mem);
    cpu->AF.A += r8;
    set_flags(cpu, (cpu->AF.A == 0), 0, (((a & 0xF) + (r8 & 0xF)) > 0xF),
              ((uint16_t)a + (uint16_t)r8) > 0xFF);
    return 1;
  }
  // adc a, r8: add the value of r8 + carry to a
  if ((ins & arithmetic_block) == 0x88) {
    uint8_t carry = cpu->AF.flags.C;
    uint8_t a = cpu->AF.A;
    uint8_t r8 = get_reg8(ins & 0b111, cpu, mem);
    cpu->AF.A = a + r8 + carry;
    set_flags(cpu, (cpu->AF.A == 0), 0,
              (((a & 0xF) + (r8 & 0xF) + carry) > 0xF),
              ((uint16_t)a + (uint16_t)r8 + carry) > 0xFF);
    return 1;
  }
  // sub a, r8: substract r8 from a
  if ((ins & arithmetic_block) == 0x90) {
    uint8_t a = cpu->AF.A;
    uint8_t r8 = get_reg8(ins & 0b111, cpu, mem);
    cpu->AF.A -= r8;
    set_flags(cpu, (cpu->AF.A == 0), 1, (r8 & 0xF) > (a & 0xF), (r8 > a));
    return 1;
  }
  // sbc a, r8: sustract r8 + carry flag from a
  if ((ins & arithmetic_block) == 0x98) {
    uint8_t carry = cpu->AF.flags.C;
    uint8_t a = cpu->AF.A;
    uint8_t r8 = get_reg8(ins & 0b111, cpu, mem);
    cpu->AF.A -= (r8 + carry);
    set_flags(cpu, (cpu->AF.A == 0), 1, ((r8 & 0xF) + carry) > (a & 0xF),
              ((r8 + carry) > a));
    return 1;
  }
  // and a, r8: set a to r8 & a
  if ((ins & arithmetic_block) == 0xA0) {
    cpu->AF.A &= get_reg8(ins & 0b111, cpu, mem);
    set_flags(cpu, (cpu->AF.A == 0), 0, 1, 0);
    return 1;
  }
  // xor a, r8: set a to r8 ^ a
  if ((ins & arithmetic_block) == 0xA8) {
    cpu->AF.A ^= get_reg8(ins & 0b111, cpu, mem);
    set_flags(cpu, (cpu->AF.A == 0), 0, 0, 0);
    return 1;
  }
  // or a, r8: set a to r8 | a
  if ((ins & arithmetic_block) == 0xB0) {
    cpu->AF.A |= get_reg8(ins & 0b111, cpu, mem);
    set_flags(cpu, (cpu->AF.A == 0), 0, 0, 0);
    return 1;
  }
  // cp a, r8: basically sub but the operation is not performed. only flags
  if ((ins & arithmetic_block) == 0xB8) {
    uint8_t a = cpu->AF.A;
    uint8_t r8 = get_reg8(ins & 0b111, cpu, mem);
    set_flags(cpu, ((a - r8) == 0), 1, (r8 & 0xF) > (a & 0xF), (r8 > a));
    return 1;
  }

  printf("ERROR: Invalid Instruction: %02X\n", ins);
  exit(-1);
  return 0;
}
