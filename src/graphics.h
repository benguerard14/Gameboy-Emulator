#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>

int graphics_init(void);

int graphics_update(void);

int graphics_exit(void);

void graphics_sleep(int ms);

void set_screen(char *screen);

void set_pixel(int x, int y, uint8_t color);

#endif
