#include "cpu.h"
#include "gameboy.h"
#include "graphics.h"
#include "timer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <stdint.h>
#include <stdio.h>

uint8_t data[16] = {0xFF, 0x00, 0x7E, 0xFF, 0x85, 0x81, 0x89, 0x83,
                    0x93, 0x85, 0xA5, 0x8B, 0xC9, 0x97, 0x7E, 0xFF};
Gameboy gb;

void loop() {
  SDL_Event event;
  while (1) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        return;
      }
    }
    uint8_t cycles;
    interrupt_handle(&gb.cpu, &gb.mem);
    if(!(gb.cpu.halted)){
      uint8_t ins = fetch_instruction(&gb.cpu, &gb.mem);
      cycles = cpu_step(ins, &gb.cpu, &gb.mem);
    }
    else{
      cycles = 1;
    }
    // printf("%02X\n", ins);
    timer_step(&gb.cpu, &gb.mem, cycles);
  }
  // free(pixels);
}

int main() {
  graphics_init();
  size_t size;
  char *rom = get_string_file("roms/cpu_instrs.gb", &size);
  emulator_init(&gb, rom, size);
  free(rom);

  loop();
  graphics_sleep(6000);
  graphics_exit();
  return 0;
}
