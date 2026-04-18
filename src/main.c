#include "graphics.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>

void loop(){
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

int main(){
  graphics_init();
  loop();
  graphics_exit();
  return 0;
}
