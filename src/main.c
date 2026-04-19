#include "emulator.h"
#include "graphics.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <stdint.h>

void loop() {
  int i = 0;
  SDL_Event event;
  while (1) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        return;
      }
    }
    set_pixel(i % 160, i / 160, 1);
    i++;
    graphics_update();
  }
}

int main() {
  graphics_init();
  char *str = get_string_file("roms/Tetris.gb");
  for (int i = 0x104; i < 0x133; i++) {
    printf("%02X ", *(uint8_t *)&str[i]);
  }
  printf("\n");
  loop();
  graphics_exit();
  return 0;
}
