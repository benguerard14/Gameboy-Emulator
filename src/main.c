#include "emulator.h"
#include "graphics.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <stdint.h>

uint8_t data[16] = {
    0xFF, 0x00, 0x7E, 0xFF,
    0x85, 0x81, 0x89, 0x83,
    0x93, 0x85, 0xA5, 0x8B,
    0xC9, 0x97, 0x7E, 0xFF
};

void loop() {
  int i = 0;
  SDL_Event event;
  uint8_t *pixels;
  pixels = pixels_from_tile(data) ;
  while (1) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        return;
      }
    }
    printf("%d ", i);
    //printf("%02b ", pixels[i]);
    set_pixel(i % 8, i / 8, pixels[i]);
    if(i > 64){
      break;
    }
    i++;
    graphics_update();
  }
  free(pixels);
}

int main() {
  graphics_init();
  char *str = get_string_file("roms/Tetris.gb");
  for (int i = 0x104; i < 0x133; i++) {
    printf("%02X ", *(uint8_t *)&str[i]);
  }
  printf("\n");
  loop();
  graphics_sleep(5000);
  graphics_exit();
  return 0;
}
