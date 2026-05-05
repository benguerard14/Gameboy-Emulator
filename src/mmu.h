#ifndef MMU_H
#define MMU_H

#include <stddef.h>
#include <stdint.h>

#define TILE_DATA_1 0x8000
#define TILE_DATA_2 0x8800

#define TILEMAP_1_ADDR 0x9800
#define TILEMAP_2_ADDR 0x9C00

#define IO_REGS_ADDR 0xFF00

#define SERIAL_DATA_OUT 0xFF01
#define SERIAL_DATA_CTRL 0xFF02

#define TIMER_DIV_ADDR 0xFF04
#define TIMER_COUNTER_ADDR 0xFF05
#define TIMER_MODULO_ADDR 0xFF06
#define TIMER_CONTROL_ADDR 0xFF07
#define INTERRUPTS_FLAG_ADDR 0xFF0F

#define LCD_CONTROL 0xFF40
#define LCD_STATUS 0xFF41
#define SCY_ADDR 0xFF42
#define SCX_ADDR 0xFF43
#define LCD_Y_COORD 0xFF44
#define LCD_Y_COMPARE 0xFF45

#define INTERRUPTS_ENABLE_ADDR 0XFFFF

typedef struct {
  uint8_t rom_bank_0[0x4000];
  uint8_t rom_bank_1[0x4000];
  uint8_t VRAM[0x2000];
  uint8_t external_RAM[0x2000];
  uint8_t WRAM[0x2000];
  uint8_t echo_ram[0x1E00];
  uint8_t OAM[0xA0];
  uint8_t no_access[0x60];
  uint8_t ioregs[0x80];
  uint8_t HRAM[0x7F];
  uint8_t interrupt_enable;
  uint8_t joypad_action;
  uint8_t joypad_direction;
  uint8_t *full_rom;
  size_t rom_size;
  uint8_t mbc_type;
  uint8_t rom_bank;
  uint8_t ram_bank;
  uint8_t ram_enabled;
  uint8_t mbc1_mode;
} __attribute__((packed)) Memory_t;

void mem_write(Memory_t *mem, uint16_t addr, uint8_t val);

uint8_t mem_read(Memory_t *mem, uint16_t addr);

void set_joypad_dir(Memory_t *mem, uint8_t bit, uint8_t pressed);

void set_joypad_action(Memory_t *mem, uint8_t bit, uint8_t pressed);

#endif
