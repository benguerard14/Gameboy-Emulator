#include "timer.h"
#include "mmu.h"
#include <stdio.h>
#include <stdint.h>

#define TIMER_CONTROL_ADDR 0xFF07
#define TIMER_COUNTER_ADDR 0xFF05
#define TIMER_MODULO_ADDR 0xFF06
#define INTERRUPTS_FLAG_ADDR 0xFF0F

void timer_step(CPU *cpu, Memory_t *mem, uint8_t m_cycles){
  cpu->div_ticks += (m_cycles * 4);
  cpu->tima_ticks += (m_cycles);

  while(cpu->div_ticks >= 256){
    mem->ioregs[0x04]++;
    cpu->div_ticks -= 256;
  }

  uint8_t tima_control = mem_read(mem, TIMER_CONTROL_ADDR);
  if(!((tima_control >> 2) & 0b1)){
    return;
  }

  uint16_t tima_freq = 0;

  switch(tima_control & 0b11){
    case 0x0:
      tima_freq = 256;
      break;
    case 0x1:
      tima_freq = 4;
      break;
    case 0x2:
      tima_freq = 16;
      break;
    case 0x3:
      tima_freq = 64;
      break;
  }

  uint16_t temp_count = mem_read(mem, TIMER_COUNTER_ADDR);
  while(cpu->tima_ticks >= tima_freq){
    temp_count++;
    if(temp_count >= 256){
      uint8_t interrupts = mem_read(mem, INTERRUPTS_FLAG_ADDR);
      mem_write(mem, INTERRUPTS_FLAG_ADDR, interrupts |= (1 << 2));
      temp_count = mem_read(mem, TIMER_MODULO_ADDR);
    }
    cpu->tima_ticks -= tima_freq;
  }
  mem_write(mem, TIMER_COUNTER_ADDR, (uint8_t)temp_count);
}
