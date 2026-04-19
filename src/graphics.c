#include "graphics.h"
#include <SDL2/SDL.h>

#define GB_WIDTH 160
#define GB_HEIGHT 144
#define SCALE 5

static uint8_t framebuffer[GB_WIDTH * GB_HEIGHT]; // 0-3 per pixel
static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;

// Game Boy palette
static const uint32_t GB_PALETTE[4] = {
    0xFF9BBC0F, // Lightest
    0xFF8BAC0F, // Light
    0xFF306230, // Dark
    0xFF0F380F,  // Darkest
};

int graphics_init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL Init Error: %s\n", SDL_GetError());
        return -1;
    }
    
    window = SDL_CreateWindow(
        "Game Boy Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        GB_WIDTH * SCALE,
        GB_HEIGHT * SCALE,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        printf("Window Creation Error: %s\n", SDL_GetError());
        return -1;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Renderer Creation Error: %s\n", SDL_GetError());
        return -1;
    }
    
    texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        GB_WIDTH,
        GB_HEIGHT
    );
    
    if (!texture) {
        printf("Texture Creation Error: %s\n", SDL_GetError());
        return -1;
    }
    
    return 0;
}

int graphics_update() {
    uint32_t pixels[GB_WIDTH * GB_HEIGHT];
    
    // Convert 2-bit color indices to ARGB
    for (int i = 0; i < GB_WIDTH * GB_HEIGHT; i++) {
        pixels[i] = GB_PALETTE[framebuffer[i] & 0x03];
    }
    
    SDL_UpdateTexture(texture, NULL, pixels, GB_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    
    return 0;
}

void set_pixel(int x, int y, uint8_t color) {
    if (x >= 0 && x < GB_WIDTH && y >= 0 && y < GB_HEIGHT) {
        framebuffer[y * GB_WIDTH + x] = color & 0x03;
    }
}

int graphics_exit() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void graphics_sleep(int ms) { SDL_Delay(ms); }
