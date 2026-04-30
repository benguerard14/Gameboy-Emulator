#include "timer.h"
#include "cpu.h"
#include "mmu.h"
#include <stdint.h>

void timer_step(CPU *cpu, Memory_t *mem, uint8_t m_cycles) {
  cpu->div_ticks += (m_cycles * 4);
  cpu->tima_ticks += (m_cycles);

  while (cpu->div_ticks >= 256) {
    mem->ioregs[TIMER_DIV_ADDR - IO_REGS_ADDR]++;
    cpu->div_ticks -= 256;
  }

  uint8_t tima_control = mem_read(mem, TIMER_CONTROL_ADDR);
  if (!((tima_control >> 2) & 0b1)) {
    return;
  }

  uint16_t tima_freq = 0;

  switch (tima_control & 0b11) {
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
  while (cpu->tima_ticks >= tima_freq) {
    temp_count++;
    if (temp_count >= 256) {
      uint8_t interrupts = mem_read(mem, INTERRUPTS_FLAG_ADDR);
      request_interrupt(mem, 2);
      temp_count = mem_read(mem, TIMER_MODULO_ADDR);
    }
    cpu->tima_ticks -= tima_freq;
  }
  mem_write(mem, TIMER_COUNTER_ADDR, (uint8_t)temp_count);
}
