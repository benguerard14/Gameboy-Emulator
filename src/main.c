#include "cpu.h"
#include "gameboy.h"
#include "graphics.h"
#include "ppu.h"
#include "timer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <stdint.h>
#include <stdio.h>

Gameboy gb;

void loop() {
  SDL_Event event;
  while (1) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        return;
      }
    }
    uint8_t cycles = 0;
    cycles = interrupt_handle(&gb.cpu, &gb.mem);
    if (cycles == 0) {
      if (!(gb.cpu.halted)) {
        uint8_t ins = fetch_instruction(&gb.cpu, &gb.mem);
        cycles = cpu_step(ins, &gb.cpu, &gb.mem);
      } else {
        cycles = 1;
      }
    }
    // printf("%02X\n", ins);
    timer_step(&gb.cpu, &gb.mem, cycles);
    ppu_step(&gb.mem, &gb.ppu, cycles);
    static int steps = 0;
  }
  // free(pixels);
}

int main() {
  graphics_init();
  size_t size;
  char *rom = get_string_file("roms/Tetris.gb", &size);
  emulator_init(&gb, rom, size);
  free(rom);

  loop();
  graphics_exit();
  return 0;
}
