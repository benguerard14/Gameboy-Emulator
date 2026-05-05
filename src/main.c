#include "cpu.h"
#include "gameboy.h"
#include "graphics.h"
#include "ppu.h"
#include "timer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_timer.h>
#include <stdint.h>
#include <stdio.h>
Gameboy gb;
void loop() {
  SDL_Event event;
  while (1) {
    uint32_t frame_start = SDL_GetTicks();
    int frame_cycles = 0;
    while (frame_cycles < 70224) {
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
          return;
        }
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
          uint8_t pressed = (event.type == SDL_KEYDOWN);
          switch (event.key.keysym.sym) {
          // Action buttons
          case SDLK_x:
            set_joypad_action(&gb.mem, 0, pressed);
            break; // A
          case SDLK_z:
            set_joypad_action(&gb.mem, 1, pressed);
            break; // B
          case SDLK_RETURN:
            set_joypad_action(&gb.mem, 3, pressed);
            break; // Start
          case SDLK_RSHIFT:
            set_joypad_action(&gb.mem, 2, pressed);
            break; // Select
          // Directional buttons
          case SDLK_RIGHT:
            set_joypad_dir(&gb.mem, 0, pressed);
            break;
          case SDLK_LEFT:
            set_joypad_dir(&gb.mem, 1, pressed);
            break;
          case SDLK_UP:
            set_joypad_dir(&gb.mem, 2, pressed);
            break;
          case SDLK_DOWN:
            set_joypad_dir(&gb.mem, 3, pressed);
            break;
          }
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
      timer_step(&gb.cpu, &gb.mem, cycles);
      ppu_step(&gb.mem, &gb.ppu, cycles);

      frame_cycles += cycles * 4;
    }

    uint32_t frame_time = SDL_GetTicks() - frame_start;
    if (frame_time < 16) {
      SDL_Delay(16 - frame_time);
    }
  }
}
int main(int argc, char *argv[]) {
  graphics_init();
  size_t size;
  char *rom = get_string_file("roms/Pokemon-Blue.gb", &size);
  emulator_init(&gb, rom, size);
  free(rom);
  loop();
  graphics_exit();
  return 0;
}
