#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include "cpu.h"
#include "mmu.h"

void timer_step(CPU *cpu, Memory_t *mem, uint8_t t_cycles);

#endif
