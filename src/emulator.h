#ifndef EMULATOR_H
#define EMULATOR_H

#include <stdint.h>

struct IORegs_t{

}__attribute__((packed));

struct Memory_t{
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
} __attribute__((packed));


#endif
