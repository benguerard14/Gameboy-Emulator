#include "cpu.h"
#include "gameboy.h"
#include "util.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t get_imm8(){
  return *fetch_instruction();
}

uint16_t get_imm16(){
    return (*fetch_instruction()) + (*fetch_instruction() << 8);
}

void set_flags(CPU *cpu, uint8_t Z, uint8_t N, uint8_t H, uint8_t C){
  cpu->AF.flags.C = C;
  cpu->AF.flags.H = H;
  cpu->AF.flags.N = N;
  cpu->AF.flags.Z = Z;
}

uint8_t get_reg8(uint8_t reg_num, CPU *cpu){
  switch(reg_num){
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
      //GET MEM POINTER HL
      return cpu->HL.val;
    case 0x7:
      //GET A
      return cpu->SP.val;
    default:
      printf("ERROR: Getting invalid reg16\n");
      exit(-1);
  }
}

void set_reg8(uint8_t reg_num, CPU *cpu, uint8_t val){

}

uint16_t get_reg16(uint8_t reg_num, CPU *cpu){
  switch(reg_num){
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

void set_reg16(uint8_t reg_num, CPU *cpu, uint16_t val){
  switch(reg_num){
    case(0x0):
      cpu->BC.val = val;
      return;
    case(0x1):
      cpu->DE.val = val;
      return;
    case(0x2):
      cpu->HL.val = val;
      return;
    case(0x3):
      cpu->SP.val = val;
      return;
    default:
      printf("ERROR: Setting invalid reg16\n");
      exit(-1);
  }
}

uint16_t get_memreg16(uint8_t reg_num, CPU *cpu){
  switch(reg_num){
    case 0x0: return cpu->BC.val;
    case 0x1: return cpu->DE.val;

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

uint8_t cpu_step(uint8_t *ins, CPU *cpu, Memory_t *mem) {
  //BLOCK 0 INSTRUCTIONS
  // nop -> do nothing
  if ((*ins & 0xFF) == 0) {
    return 1;
  }
  uint8_t block0_mask = 0b11001111;
  uint8_t block0_sel = (*ins >> 4) & 0b11;
  // ld r16, imm16
  if ((*ins & block0_mask) == 0x1) {
    uint16_t val = get_imm16();
    set_reg16(block0_sel, cpu, val);
    return 3;
  }
  //LD [r16_mem],A
  if((*ins & block0_mask) == 0x2){
    uint16_t addr = get_memreg16(block0_sel, cpu);
    mem_write(mem, addr, cpu->AF.A);
    return 2;
  }
  //LD A,[r16_mem]
  if((*ins & block0_mask) == 0xA){
    uint16_t addr = get_memreg16(block0_sel, cpu);
    uint8_t val = mem_read(mem, addr);
    cpu->AF.A = val;
    return 2;
  }
  //ld [imm16], sp
  if(*ins == 0x8){
    uint16_t addr = get_imm16();
    mem_write(mem, addr, cpu->SP.lo);
    mem_write(mem, addr + 1, cpu->SP.hi);
    return 5;
  }

  //inc r16
  if((*ins & block0_mask) == 0x3){
    uint16_t val = get_reg16(block0_sel, cpu);
    set_reg16(block0_sel, cpu, val + 1);
    return 2;
  }
  //dec r16
  if((*ins & block0_mask) == 0xB){
    uint16_t val = get_reg16(block0_sel, cpu);
    set_reg16(block0_sel, cpu, val + 1);
    return 2;
  }
  //add hl, r16
  if((*ins & block0_mask) == 0x9){
    uint16_t val = get_reg16(block0_sel, cpu);
    uint32_t sum = val + cpu->HL.val;
    set_flags(cpu, cpu->AF.flags.Z, 0, ((cpu->HL.val & 0x0FFF) + (val & 0x0FFF)) > 0x0FFF, (sum > 0xFFFF));
    cpu->HL.val = (uint16_t)sum;
    return 2;
  }

  //inc r8
  

  printf("ERROR: Invalid Instruction: %02X\n", *ins);
  exit(-1);
  return 0;
}
