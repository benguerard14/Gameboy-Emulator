#include "cpu.h"
#include "mmu.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t fetch_instruction(CPU *cpu, Memory_t *mem) {
  //EI is one step later
  if(cpu->IME_next){
    cpu->IME = 1;
    cpu->IME_next = 0;
  }
  uint8_t ins;
  ins = mem_read(mem, cpu->PC.val);
  cpu->PC.val++;
  return ins;
}

uint8_t get_imm8(CPU *cpu, Memory_t *mem) {
  return fetch_instruction(cpu, mem);
}

uint16_t get_imm16(CPU *cpu, Memory_t *mem) {
  return (fetch_instruction(cpu, mem)) + (fetch_instruction(cpu, mem) << 8);
}

void push_16(CPU *cpu, Memory_t *mem, uint16_t val) {
  cpu->SP.val--;
  mem_write(mem, cpu->SP.val, val >> 8);
  cpu->SP.val--;
  mem_write(mem, cpu->SP.val, val & 0xFF);
}

uint16_t pop_16(CPU *cpu, Memory_t *mem) {
  uint16_t val = 0;
  val |= mem_read(mem, cpu->SP.val);
  cpu->SP.val++;
  val |= (mem_read(mem, cpu->SP.val)) << 8;
  cpu->SP.val++;
  return val;
}

void enable_interrupts(CPU *cpu) { cpu->IME_next = 1; }
void disable_interrupts(CPU *cpu) { cpu->IME = 0;}

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

uint8_t get_reg8(uint8_t reg_num, CPU *cpu, Memory_t *mem, uint8_t *adj) {
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
    *adj += 1;
    return mem_read(mem, cpu->HL.val);
  case 0x7:
    // GET A
    return cpu->AF.A;
  default:
    printf("ERROR: Getting invalid reg8\n");
    exit(-1);
  }
}

void set_reg8(uint8_t reg_num, CPU *cpu, Memory_t *mem, uint8_t val,
              uint8_t *adj) {
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
    *adj += 1;
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

uint16_t get_reg16stk(uint8_t reg_num, CPU *cpu) {
  switch (reg_num) {
  case 0x0:
    return cpu->BC.val;
  case 0x1:
    return cpu->DE.val;
  case 0x2:
    return cpu->HL.val;
  case 0x3:
    return cpu->AF.AF;
  default:
    printf("ERROR: Getting invalid reg16\n");
    exit(-1);
  }
}

void set_reg16stk(uint8_t reg_num, CPU *cpu, uint16_t val) {
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
    cpu->AF.AF = val & 0xFFF0;
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

uint8_t interrupt_handle(CPU *cpu, Memory_t *mem){
  uint8_t IE = mem_read(mem, 0XFFFF);
  uint8_t IF = mem_read(mem, 0xFF0F);
  uint8_t pending = IE & IF;
  
  if(pending){
    cpu->halted = 0;
  }

  if (!pending || !cpu->IME) {
    return 0;
  }
  
  uint16_t addr;
  uint8_t bits;
  if(pending & 0b1){
    addr = 0x40;
    bits = 0;
  }
  else if((pending >> 1) & 0b1){
    addr = 0x48;
    bits = 1;
  }
  else if((pending >> 2) & 0b1){
    addr = 0x50;
    bits = 2;
  }
  else if((pending >> 3) & 0b1){
    addr = 0x58;
    bits = 3;
  }
  else if((pending >> 4) & 0b1){
    addr = 0x60;
    bits = 4;
  }
  else{
    printf("ERROR: Invalid Interrupt");
    exit(-1);
  }

  cpu->IME = 0;
  cpu->IME_next = 0;

  mem_write(mem, 0xFF0F, IF & ~(1 << bits));
  push_16(cpu, mem, cpu->PC.val);
  cpu->PC.val = addr;
  return 5;
}

uint8_t cb_instruction(uint8_t ins, CPU *cpu, Memory_t *mem) {
  uint8_t adj = 0;
  uint8_t cb_bitmask = 0b11111000;
  // rlc r8: rotate r8 to the left
  if ((ins & cb_bitmask) == 0x0) {
    uint8_t val = get_reg8(ins & 0b111, cpu, mem, &adj);
    uint8_t carry = (val >> 7) & 0b1;
    uint8_t result = (val << 1) | carry;
    set_flags(cpu, result == 0, 0, 0, carry);
    set_reg8(ins & 0b111, cpu, mem, result, &adj);
    return 2 + adj;
  }
  // rrc r8: rotate r8 to the right
  if ((ins & cb_bitmask) == 0x08) {
    uint8_t val = get_reg8(ins & 0b111, cpu, mem, &adj);
    uint8_t carry = val & 0b1;
    uint8_t result = (val >> 1) | (carry << 7);
    set_flags(cpu, result == 0, 0, 0, carry);
    set_reg8(ins & 0b111, cpu, mem, result, &adj);
    return 2 + adj;
  }
  // rl r8: rotate r8 left through the carry flag
  if ((ins & cb_bitmask) == 0x10) {
    uint8_t val = get_reg8(ins & 0b111, cpu, mem, &adj);
    uint8_t carry = (val >> 7) & 0b1;
    uint8_t result = (val << 1) | cpu->AF.flags.C;
    set_flags(cpu, result == 0, 0, 0, carry);
    set_reg8(ins & 0b111, cpu, mem, result, &adj);
    return 2 + adj;
  }
  // rr r8: rotate r8 right through the carry flag
  if ((ins & cb_bitmask) == 0x18) {
    uint8_t val = get_reg8(ins & 0b111, cpu, mem, &adj);
    uint8_t carry = val & 0b1;
    uint8_t result = (val >> 1) | (cpu->AF.flags.C << 7);
    set_flags(cpu, result == 0, 0, 0, carry);
    set_reg8(ins & 0b111, cpu, mem, result, &adj);
    return 2 + adj;
  }
  // sla r8: shift r8 left arithmetically
  if ((ins & cb_bitmask) == 0x20) {
    uint8_t val = get_reg8(ins & 0b111, cpu, mem, &adj);
    uint8_t carry = (val >> 7) & 0b1;
    uint8_t result = val << 1;
    set_flags(cpu, result == 0, 0, 0, carry);
    set_reg8(ins & 0b111, cpu, mem, result, &adj);
    return 2 + adj;
  }
  // sra r8: shift r8 right arithmetically
  if ((ins & cb_bitmask) == 0x28) {
    uint8_t val = get_reg8(ins & 0b111, cpu, mem, &adj);
    uint8_t carry = val & 0b1;
    uint8_t result = (val >> 1) | (val & 0x80);
    set_flags(cpu, result == 0, 0, 0, carry);
    set_reg8(ins & 0b111, cpu, mem, result, &adj);
    return 2 + adj;
  }
  // swap r8: swap upper and lower 4 bits
  if ((ins & cb_bitmask) == 0x30) {
    uint8_t val = get_reg8(ins & 0b111, cpu, mem, &adj);
    uint8_t result = ((val & 0xF) << 4) | (val >> 4);
    set_flags(cpu, result == 0, 0, 0, 0);
    set_reg8(ins & 0b111, cpu, mem, result, &adj);
    return 2 + adj;
  }
  // srl r8: shift r8 right logically
  if ((ins & cb_bitmask) == 0x38) {
    uint8_t val = get_reg8(ins & 0b111, cpu, mem, &adj);
    uint8_t carry = val & 0b1;
    uint8_t result = val >> 1;
    set_flags(cpu, result == 0, 0, 0, carry);
    set_reg8(ins & 0b111, cpu, mem, result, &adj);
    return 2 + adj;
  }

  // bit b3, r8: Test bit u3 in register r8, set the zero flag if bit not set.
  uint8_t majoras_mask = 0b11000000;
  if ((ins & majoras_mask) == 0x40) {
    uint8_t val = get_reg8(ins & 0b111, cpu, mem, &adj);
    uint8_t bit = (ins >> 3) & 0b111;
    uint8_t z = ((val >> bit) & 0b1) == 0;
    set_flags(cpu, z, 0, 1, cpu->AF.flags.C);
    return 2 + adj;
  }
  // res b3, r8: Set bit u3 in register r8 to 0.
  if ((ins & majoras_mask) == 0x80) {
    uint8_t val = get_reg8(ins & 0b111, cpu, mem, &adj);
    uint8_t bit = (ins >> 3) & 0b111;
    uint8_t result = val & ~(1 << bit);
    set_reg8(ins & 0b111, cpu, mem, result, &adj);
    return 2 + adj;
  }
  // set b3, r8: Set bit u3 in register r8 to 1.
  if ((ins & majoras_mask) == 0xC0) {
    uint8_t val = get_reg8(ins & 0b111, cpu, mem, &adj);
    uint8_t bit = (ins >> 3) & 0b111;
    uint8_t result = val | (1 << bit);
    set_reg8(ins & 0b111, cpu, mem, result, &adj);
    return 2 + adj;
  }

  printf("ERROR: Invalid CB instruction\n");
  exit(-1);
  return 0;
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
    uint16_t val = get_imm16(cpu, mem);
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
    uint16_t addr = get_imm16(cpu, mem);
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
    uint8_t adj = 0;
    uint8_t val = get_reg8(block0_8sel, cpu, mem, &adj);
    uint8_t result = val + 1;
    set_reg8(block0_8sel, cpu, mem, result, &adj);
    set_flags(cpu, (result) == 0, 0, (val & 0x0F) == 0x0F, cpu->AF.flags.C);
    return 1 + adj;
  }
  // dec r8
  if ((ins & block0_8mask) == 0x5) {
    uint8_t adj = 0;
    uint8_t val = get_reg8(block0_8sel, cpu, mem, &adj);
    uint8_t result = val - 1;
    set_reg8(block0_8sel, cpu, mem, result, &adj);
    set_flags(cpu, (result) == 0, 1, (val & 0x0F) == 0x0, cpu->AF.flags.C);
    return 1 + adj;
  }

  // ld r8, n8
  if ((ins & block0_8mask) == 0x6) {
    uint8_t adj = 0;
    uint8_t val = get_imm8(cpu, mem);
    set_reg8(block0_8sel, cpu, mem, val, &adj);
    return 2 + adj;
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
    cpu->PC.val += (int8_t)get_imm8(cpu, mem);
    return 3;
  }

  // jr cc, imm8: jump by offsetting if cc
  if ((ins & condition_mask) == 0x20) {
    int8_t addr = (int8_t)get_imm8(cpu, mem);
    if (get_condition((ins >> 3) & 0b11, cpu)) {
      cpu->PC.val += addr;
      return 3;
    }
    return 2;
  }

  // stop: no idea but looks even worse than halt
  if (ins == 0x10) {
    get_imm8(cpu, mem);
    //cpu->halted = 1;
    // printf("Stop instruction. No idea what to do\n");

    return 2;
  }

  // BLOCK 1
  uint8_t reg_mask = 0b11000000;
  //  halt: absolutely no idea what it does and hope i don't have to find out
  if (ins == 0x76) {
    cpu->halted = 1;
    // printf("Halt instruction. No idea what to do\n");
    return 1;
  }
  // ld r8, r8
  if ((ins & reg_mask) == 0x40) {
    uint8_t adj = 0;
    uint8_t val = get_reg8(ins & 0b111, cpu, mem, &adj);
    set_reg8((ins >> 3) & 0b111, cpu, mem, val, &adj);
    return 1 + adj;
  }

  // BLOCK 2
  uint8_t arithmetic_block = 0b11111000;
  // add a, r8: add the value of r8 to a
  if ((ins & arithmetic_block) == 0x80) {
    uint8_t adj = 0;
    uint8_t a = cpu->AF.A;
    uint8_t r8 = get_reg8(ins & 0b111, cpu, mem, &adj);
    cpu->AF.A += r8;
    set_flags(cpu, (cpu->AF.A == 0), 0, (((a & 0xF) + (r8 & 0xF)) > 0xF),
              ((uint16_t)a + (uint16_t)r8) > 0xFF);
    return 1 + adj;
  }
  // adc a, r8: add the value of r8 + carry to a
  if ((ins & arithmetic_block) == 0x88) {
    uint8_t adj = 0;
    uint8_t carry = cpu->AF.flags.C;
    uint8_t a = cpu->AF.A;
    uint8_t r8 = get_reg8(ins & 0b111, cpu, mem, &adj);
    cpu->AF.A = a + r8 + carry;
    set_flags(cpu, (cpu->AF.A == 0), 0,
              (((a & 0xF) + (r8 & 0xF) + carry) > 0xF),
              ((uint16_t)a + (uint16_t)r8 + carry) > 0xFF);
    return 1 + adj;
  }
  // sub a, r8: substract r8 from a
  if ((ins & arithmetic_block) == 0x90) {
    uint8_t adj = 0;
    uint8_t a = cpu->AF.A;
    uint8_t r8 = get_reg8(ins & 0b111, cpu, mem, &adj);
    cpu->AF.A -= r8;
    set_flags(cpu, (cpu->AF.A == 0), 1, (r8 & 0xF) > (a & 0xF), (r8 > a));
    return 1 + adj;
  }
  // sbc a, r8: sustract r8 + carry flag from a
  if ((ins & arithmetic_block) == 0x98) {
    uint8_t adj = 0;
    uint8_t carry = cpu->AF.flags.C;
    uint8_t a = cpu->AF.A;
    uint8_t r8 = get_reg8(ins & 0b111, cpu, mem, &adj);
    uint16_t full = (uint16_t)a - (uint16_t)r8 - carry;
    cpu->AF.A = (uint8_t)full;
    set_flags(cpu, (cpu->AF.A == 0), 1,
              ((int)(a & 0xF) - (int)(r8 & 0xF) - carry) < 0, full > 0xFF);
    return 1 + adj;
  }
  // and a, r8: set a to r8 & a
  if ((ins & arithmetic_block) == 0xA0) {
    uint8_t adj = 0;
    cpu->AF.A &= get_reg8(ins & 0b111, cpu, mem, &adj);
    set_flags(cpu, (cpu->AF.A == 0), 0, 1, 0);
    return 1 + adj;
  }
  // xor a, r8: set a to r8 ^ a
  if ((ins & arithmetic_block) == 0xA8) {
    uint8_t adj = 0;
    cpu->AF.A ^= get_reg8(ins & 0b111, cpu, mem, &adj);
    set_flags(cpu, (cpu->AF.A == 0), 0, 0, 0);
    return 1 + adj;
  }
  // or a, r8: set a to r8 | a
  if ((ins & arithmetic_block) == 0xB0) {
    uint8_t adj = 0;
    cpu->AF.A |= get_reg8(ins & 0b111, cpu, mem, &adj);
    set_flags(cpu, (cpu->AF.A == 0), 0, 0, 0);
    return 1 + adj;
  }
  // cp a, r8: basically sub but the operation is not performed. only flags
  if ((ins & arithmetic_block) == 0xB8) {
    uint8_t adj = 0;
    uint8_t a = cpu->AF.A;
    uint8_t r8 = get_reg8(ins & 0b111, cpu, mem, &adj);
    set_flags(cpu, ((a - r8) == 0), 1, (r8 & 0xF) > (a & 0xF), (r8 > a));
    return 1 + adj;
  }

  // BLOCK 3
  //  add a, imm8: add the value of imm8 to a
  if (ins == 0xC6) {
    uint8_t a = cpu->AF.A;
    uint8_t imm8 = get_imm8(cpu, mem);
    cpu->AF.A += imm8;
    set_flags(cpu, (cpu->AF.A == 0), 0, (((a & 0xF) + (imm8 & 0xF)) > 0xF),
              ((uint16_t)a + (uint16_t)imm8) > 0xFF);
    return 2;
  }
  // adc aimm imm8: add the value of imm8 + carry to a
  if (ins == 0xCE) {
    uint8_t carry = cpu->AF.flags.C;
    uint8_t a = cpu->AF.A;
    uint8_t imm8 = get_imm8(cpu, mem);
    cpu->AF.A = a + imm8 + carry;
    set_flags(cpu, (cpu->AF.A == 0), 0,
              (((a & 0xF) + (imm8 & 0xF) + carry) > 0xF),
              ((uint16_t)a + (uint16_t)imm8 + carry) > 0xFF);
    return 2;
  }
  // sub a, imm8: substract imm8 from a
  if (ins == 0xD6) {
    uint8_t a = cpu->AF.A;
    uint8_t imm8 = get_imm8(cpu, mem);
    cpu->AF.A -= imm8;
    set_flags(cpu, (cpu->AF.A == 0), 1, (imm8 & 0xF) > (a & 0xF), (imm8 > a));
    return 2;
  }
  // sbc a, imm8: sustract imm8 + carry flag from a
  if (ins == 0xDE) {
    uint8_t carry = cpu->AF.flags.C;
    uint8_t a = cpu->AF.A;
    uint8_t imm8 = get_imm8(cpu, mem);
    uint16_t full = (uint16_t)a - (uint16_t)imm8 - carry;
    cpu->AF.A = (uint8_t)full;
    set_flags(cpu, (cpu->AF.A == 0), 1,
              ((int)(a & 0xF) - (int)(imm8 & 0xF) - carry) < 0, full > 0xFF);
    return 2;
  }
  // and a, imm8: set a to imm8 & a
  if (ins == 0xE6) {
    cpu->AF.A &= get_imm8(cpu, mem);
    set_flags(cpu, (cpu->AF.A == 0), 0, 1, 0);
    return 2;
  }
  // xor a, imm8: set a to imm8 ^ a
  if (ins == 0xEE) {
    cpu->AF.A ^= get_imm8(cpu, mem);
    set_flags(cpu, (cpu->AF.A == 0), 0, 0, 0);
    return 2;
  }
  // or a, imm8: set a to imm8 | a
  if (ins == 0xF6) {
    cpu->AF.A |= get_imm8(cpu, mem);
    set_flags(cpu, (cpu->AF.A == 0), 0, 0, 0);
    return 2;
  }
  // cp a, imm8: basically sub but the operation is not performed. only flags
  if (ins == 0xFE) {
    uint8_t a = cpu->AF.A;
    uint8_t imm8 = get_imm8(cpu, mem);
    set_flags(cpu, ((a - imm8) == 0), 1, (imm8 & 0xF) > (a & 0xF), (imm8 > a));
    return 2;
  }

  // ret cc, return if cc is met
  if ((ins & condition_mask) == 0xC0) {
    if (get_condition((ins >> 3) & 0b11, cpu)) {
      cpu->PC.val = pop_16(cpu, mem);
      return 5;
    }
    return 2;
  }
  // ret: pc = pop16
  if (ins == 0xC9) {
    cpu->PC.val = pop_16(cpu, mem);
    return 4;
  }
  // reti: return and then enable inputs
  if (ins == 0xD9) {
    cpu->PC.val = pop_16(cpu, mem);
    cpu->IME = 1;
    return 4;
  }
  // jp cc, imm16: copy imm16 into PC if condition
  if ((ins & condition_mask) == 0xC2) {
    uint16_t addr = get_imm16(cpu, mem);
    if (get_condition((ins >> 3) & 0b11, cpu)) {
      cpu->PC.val = addr;
      return 4;
    }
    return 3;
  }
  // jp imm16: unconditionally copy imm16 to PC
  if (ins == 0xC3) {
    cpu->PC.val = get_imm16(cpu, mem);
    return 4;
  }
  // jp hl: copy value hl in pc
  if (ins == 0xE9) {
    cpu->PC.val = cpu->HL.val;
    return 1;
  }
  // call cc, imm16: call addr imm16 if cc. call means subroutine
  if ((ins & condition_mask) == 0xC4) {
    uint16_t addr = get_imm16(cpu, mem);
    if (get_condition((ins >> 3) & 0b11, cpu)) {
      push_16(cpu, mem, cpu->PC.val);
      cpu->PC.val = addr;
      return 6;
    }
    return 3;
  }
  // call imm16: call imm16{
  if (ins == 0xCD) {
    uint16_t addr = get_imm16(cpu, mem);
    push_16(cpu, mem, cpu->PC.val);
    cpu->PC.val = addr;
    return 6;
  }

  // rst VEC: set pc to vc
  if ((ins & block0_8mask) == 0xC7) {
    uint16_t addr = ins & 0x38;
    push_16(cpu, mem, cpu->PC.val);
    cpu->PC.val = addr;
    return 4;
  }

  // pop r16 stk: pop from the stack and store it in r16
  if ((ins & block0_16mask) == 0xC1) {
    uint16_t val = pop_16(cpu, mem);
    set_reg16stk((ins >> 4 & 0b11), cpu, val);
    return 3;
  }

  // push r16 stk: push the value in r16 to the stack
  if ((ins & block0_16mask) == 0xC5) {
    uint16_t val = get_reg16stk((ins >> 4) & 0b11, cpu);
    push_16(cpu, mem, val);
    return 4;
  }

  // CB instruction
  if (ins == 0xCB) {
    uint8_t steps = cb_instruction(get_imm8(cpu, mem), cpu, mem);
    return steps;
  }

  // ldh [c], a: Copy the value in register A into the byte at address $FF00+C.
  if (ins == 0xE2) {
    mem_write(mem, 0xFF00 + cpu->BC.lo, cpu->AF.A);
    return 2;
  }
  // ldh [imm8], a: Copy the value in register A into the byte at address
  // $FF00+imm8.
  if (ins == 0xE0) {
    uint8_t val = get_imm8(cpu, mem);
    mem_write(mem, 0xFF00 + val, cpu->AF.A);
    return 3;
  }
  // ld [imm16], a: Copy the value in register A into the byte at address imm16.
  if (ins == 0xEA) {
    mem_write(mem, get_imm16(cpu, mem), cpu->AF.A);
    return 4;
  }

  // ldh a, [c]: Copy the value in register [FF00 + c] into reg8 A
  if (ins == 0xF2) {
    uint8_t val = mem_read(mem, 0xFF00 + cpu->BC.lo);
    cpu->AF.A = val;
    return 2;
  }
  // ldh a, [imm8]: Copy the value in register [FF00 + imm8] into reg8 A
  if (ins == 0xF0) {
    uint8_t val = mem_read(mem, 0xFF00 + get_imm8(cpu, mem));
    cpu->AF.A = val;
    return 3;
  }
  // ld a, [imm16]: Copy the value in register [imm16] into reg8 A
  if (ins == 0xFA) {
    uint8_t val = mem_read(mem, get_imm16(cpu, mem));
    cpu->AF.A = val;
    return 4;
  }

  // add sp, imm8: add signed value imm8 to sp
  if (ins == 0xE8) {
    uint8_t e8 = get_imm8(cpu, mem);
    int8_t offset = (int8_t)e8;
    uint16_t sp = cpu->SP.val;

    uint16_t result = sp + offset;
    uint8_t h = ((sp & 0xF) + (offset & 0xF)) > 0xF;

    uint8_t c = ((sp & 0xFF) + (uint8_t)offset) > 0xFF;

    cpu->SP.val = result;
    set_flags(cpu, 0, 0, h, c);
    return 4;
  }

  // LD HL,SP+e8: Add the signed value e8 to SP and copy the result in HL.
  if (ins == 0xF8) {
    uint8_t e8 = get_imm8(cpu, mem);
    int8_t offset = (int8_t)e8;
    uint16_t sp = cpu->SP.val;

    uint16_t result = sp + offset;

    uint8_t h = ((sp & 0xF) + (offset & 0xF)) > 0xF;

    uint8_t c = ((sp & 0xFF) + (uint8_t)offset) > 0xFF;

    cpu->HL.val = result;
    set_flags(cpu, 0, 0, h, c);
    return 3;
  }
  // LD SP,HL Copy register HL into register SP.
  if (ins == 0xF9) {
    cpu->SP.val = cpu->HL.val;
    return 2;
  }

  if (ins == 0xF3) {
    disable_interrupts(cpu);
    return 1;
  }
  if (ins == 0xFB) {
    enable_interrupts(cpu);
    return 1;
  }

  printf("ERROR: Invalid Instruction: %02X\n", ins);
  exit(-1);
}
