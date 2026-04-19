#include "gameboy.h"
#include "graphics.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <stdint.h>

uint8_t data[16] = {0xFF, 0x00, 0x7E, 0xFF, 0x85, 0x81, 0x89, 0x83,
                    0x93, 0x85, 0xA5, 0x8B, 0xC9, 0x97, 0x7E, 0xFF};

void loop() {
  int i = 0;
  SDL_Event event;
  uint8_t *pixels;
  pixels = pixels_from_tile(data);
  while (i < 64) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        return;
      }
    }
    set_pixel(i % 8, i / 8, pixels[i]);
    i++;
    graphics_update();
  }
  free(pixels);
}

int main() {
  graphics_init();
  size_t size;
  char *rom = get_string_file("roms/Tetris.gb", &size);
  emulator_init(rom, size);
  free(rom);

  loop();
  graphics_sleep(1000);
  graphics_exit();
  return 0;
}
