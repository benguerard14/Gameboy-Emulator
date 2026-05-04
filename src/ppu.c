#include "ppu.h"
#include "cpu.h"
#include "graphics.h"
#include "mmu.h"
#include <stdio.h>
#include <stdlib.h>

uint8_t *pixels_from_tile(uint16_t tile[16]) {
  uint8_t *pixels = malloc(64 * sizeof(uint8_t));

  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      uint8_t bit1 = (tile[i * 2] >> j) & 0x01;
      uint8_t bit2 = ((tile[(i * 2) + 1] >> j) & 0x01) << 1;
      pixels[(7 - j) + 8 * i] = bit1 + bit2;
    }
  }
  return pixels;
}

void check_line(Memory_t *mem, PPU *ppu) {
  uint8_t stat = mem_read(mem, LCD_STATUS);
  mem_write(mem, LCD_Y_COORD, ppu->lines);
  if (mem_read(mem, LCD_Y_COORD) == mem_read(mem, LCD_Y_COMPARE)) {
    if (!ppu->stat_interrupted) {
      mem_write(mem, LCD_STATUS, stat | (1 << 2));
      ppu->stat_interrupted = 1;
      if (stat & (1 << 6)) {
        request_interrupt(mem, STAT_INTERRUPT);
      }
    }
  } else {
    mem_write(mem, LCD_STATUS, stat & ~(1 << 2));
    ppu->stat_interrupted = 0;
  }
}

void render_scanline(Memory_t *mem, PPU *ppu) {
  uint8_t lcdc = mem_read(mem, LCD_CONTROL);
  uint8_t scy = mem_read(mem, SCY_ADDR);
  uint8_t scx = mem_read(mem, SCX_ADDR);
  uint8_t bgp = mem_read(mem, 0xFF47);
  uint8_t y = ppu->lines;
  uint8_t bg_colors[160] = {0};

  // Background
  if (lcdc & (1 << 0)) {
    uint8_t bg_y = (y + scy) % 256;
    uint16_t tilemap_base = (lcdc & (1 << 3)) ? 0x9C00 : 0x9800;

    for (uint8_t pixel_count = 0; pixel_count < 160;) {
      uint8_t bg_x = (pixel_count + scx) % 256;
      uint16_t map_addr = tilemap_base + (bg_y / 8) * 32 + (bg_x / 8);
      uint8_t tile_id = mem_read(mem, map_addr);

      uint16_t tile_addr;
      if (lcdc & (1 << 4)) {
        tile_addr = 0x8000 + tile_id * 16;
      } else {
        tile_addr = (uint16_t)(0x9000 + (int8_t)tile_id * 16);
      }

      uint8_t tile_row = bg_y % 8;
      uint8_t lo = mem_read(mem, tile_addr + tile_row * 2);
      uint8_t hi = mem_read(mem, tile_addr + tile_row * 2 + 1);

      uint8_t col_start = bg_x % 8;
      for (uint8_t col = col_start; col < 8 && pixel_count < 160; col++) {
        uint8_t bit = 7 - col;
        uint8_t raw = ((lo >> bit) & 0x1) | (((hi >> bit) & 0x1) << 1);
        uint8_t color = (bgp >> (raw * 2)) & 0x03;
        bg_colors[pixel_count] = color;
        set_pixel(pixel_count, y, color);
        pixel_count++;
      }
    }
  }

  // Window
  uint8_t wy = mem_read(mem, 0xFF4A);
  uint8_t wx = mem_read(mem, 0xFF4B);

  if ((lcdc & (1 << 5)) && y >= wy && wx <= 166) {
    uint16_t win_tilemap_base = (lcdc & (1 << 6)) ? 0x9C00 : 0x9800;
    uint8_t win_y = ppu->window_line;
    uint8_t win_x0 = (wx < 7) ? 0 : wx - 7;

    for (uint8_t pixel_count = win_x0; pixel_count < 160;) {
      uint8_t col_in_win = pixel_count - win_x0;
      uint16_t map_addr =
          win_tilemap_base + (win_y / 8) * 32 + (col_in_win / 8);
      uint8_t tile_id = mem_read(mem, map_addr);

      uint16_t tile_addr;
      if (lcdc & (1 << 4)) {
        tile_addr = 0x8000 + tile_id * 16;
      } else {
        tile_addr = (uint16_t)(0x9000 + (int8_t)tile_id * 16);
      }

      uint8_t tile_row = win_y % 8;
      uint8_t lo = mem_read(mem, tile_addr + tile_row * 2);
      uint8_t hi = mem_read(mem, tile_addr + tile_row * 2 + 1);

      uint8_t col_start = col_in_win % 8;
      for (uint8_t col = col_start; col < 8 && pixel_count < 160; col++) {
        uint8_t bit = 7 - col;
        uint8_t raw = ((lo >> bit) & 0x1) | (((hi >> bit) & 0x1) << 1);
        uint8_t color = (bgp >> (raw * 2)) & 0x03;
        bg_colors[pixel_count] = color;
        set_pixel(pixel_count, y, color);
        pixel_count++;
      }
    }
    ppu->window_line++;
  }

  // Sprites
  if (lcdc & (1 << 1)) {
    uint8_t sprite_height = (lcdc & (1 << 2)) ? 16 : 8;

    typedef struct {
      uint8_t yi, xi, tile, flags;
    } Sprite;
    Sprite visible[10];
    uint8_t count = 0;

    for (uint8_t s = 0; s < 40 && count < 10; s++) {
      uint16_t oam = 0xFE00 + s * 4;
      uint8_t sy = mem_read(mem, oam + 0);
      uint8_t sx = mem_read(mem, oam + 1);
      uint8_t tile = mem_read(mem, oam + 2);
      uint8_t flags = mem_read(mem, oam + 3);

      if (y + 16 >= sy && y + 16 < sy + sprite_height) {
        visible[count++] = (Sprite){sy, sx, tile, flags};
      }
    }

    for (int s = count - 1; s >= 0; s--) {
      Sprite *sp = &visible[s];

      uint8_t obp = (sp->flags & (1 << 4)) ? mem_read(mem, 0xFF49)
                                           : mem_read(mem, 0xFF48);
      uint8_t flip_y = sp->flags & (1 << 6);
      uint8_t flip_x = sp->flags & (1 << 5);
      uint8_t bg_pri = sp->flags & (1 << 7);

      uint8_t sprite_row = y + 16 - sp->yi;
      if (flip_y)
        sprite_row = sprite_height - 1 - sprite_row;

      uint8_t tile_id = (sprite_height == 16) ? (sp->tile & 0xFE) : sp->tile;
      uint16_t tile_addr = 0x8000 + tile_id * 16 + sprite_row * 2;

      uint8_t lo = mem_read(mem, tile_addr);
      uint8_t hi = mem_read(mem, tile_addr + 1);

      for (uint8_t col = 0; col < 8; col++) {
        int screen_x = (int)sp->xi - 8 + col;
        if (screen_x < 0 || screen_x >= 160)
          continue;

        uint8_t bit = flip_x ? col : (7 - col);
        uint8_t raw = ((lo >> bit) & 0x1) | (((hi >> bit) & 0x1) << 1);
        if (raw == 0)
          continue;

        uint8_t color = (obp >> (raw * 2)) & 0x03;

        if (bg_pri && bg_colors[screen_x] != 0)
          continue;

        set_pixel(screen_x, y, color);
      }
    }
  }
}

void ppu_step(Memory_t *mem, PPU *ppu, uint8_t cycles) {
  static int frame_count = 0;
  static int step_count = 0;
  step_count++;
  if (ppu->lines == 144 && ppu->mode == 1)
    frame_count++;

  ppu->dots += cycles * 4;
  while (1) {
    if (!(mem_read(mem, LCD_CONTROL) & (1 << 7))) {
      ppu->dots = 0;
      ppu->lines = 0;
      ppu->stat_interrupted = 0;
      ppu->mode = 0;
      mem_write(mem, LCD_Y_COORD, 0);

      uint8_t stat = mem_read(mem, LCD_STATUS);
      stat = (stat & ~0x03);
      mem_write(mem, LCD_STATUS, stat);

      return;
    }
    if (ppu->mode == 2) {
      if (ppu->dots < 80)
        break;

      ppu->dots -= 80;

      ppu->mode = 3;

      uint8_t stat = mem_read(mem, LCD_STATUS);
      stat &= ~(0x3);
      mem_write(mem, LCD_STATUS, stat |= 0x3);
    }

    else if (ppu->mode == 3) {
      if (ppu->dots < 172 + ppu->mode3_extra)
        break;

      ppu->dots -= (172 + ppu->mode3_extra);

      render_scanline(mem, ppu);

      ppu->mode = 0;

      uint8_t stat = mem_read(mem, LCD_STATUS);
      stat &= ~(0x3);
      mem_write(mem, LCD_STATUS, stat);

      if (stat & (1 << 3))
        request_interrupt(mem, STAT_INTERRUPT);
    }

    else if (ppu->mode == 0) {
      if (ppu->dots < 456 - (252 + ppu->mode3_extra))
        break;

      ppu->dots -= (456 - (252 + ppu->mode3_extra));
      ppu->mode3_extra = 0;

      ppu->lines = (ppu->lines + 1) % 154;
      check_line(mem, ppu);
      if (ppu->lines == 144) {
        printf("LY=%d IF=%02X IE=%02X \n", mem_read(mem, 0xFF44),
               mem_read(mem, 0xFF0F), mem_read(mem, 0xFFFF));
        ppu->mode = 1;
        uint8_t stat = mem_read(mem, LCD_STATUS);
        stat &= ~(0x3);
        mem_write(mem, LCD_STATUS, stat |= 0x1);
        graphics_update();
        request_interrupt(mem, VBLANK_INTERRUPT);

        if (stat & (1 << 4))
          request_interrupt(mem, STAT_INTERRUPT);
      } else {
        ppu->mode = 2;
        uint8_t stat = mem_read(mem, LCD_STATUS);
        stat &= ~(0x3);
        mem_write(mem, LCD_STATUS, stat |= 0x2);

        if (stat & (1 << 5))
          request_interrupt(mem, STAT_INTERRUPT);
      }
    }

    else if (ppu->mode == 1) {
      if (ppu->dots < 456)
        break;

      ppu->dots -= 456;
      ppu->lines = (ppu->lines + 1) % 154;
      check_line(mem, ppu);

      if (ppu->lines == 0) {
        ppu->mode = 2;
        ppu->window_line = 0;
        uint8_t stat = mem_read(mem, LCD_STATUS);
        stat &= ~(0x3);
        mem_write(mem, LCD_STATUS, stat |= 0x2);

        if (stat & (1 << 5))
          request_interrupt(mem, STAT_INTERRUPT);
      }
    }
  }
}
