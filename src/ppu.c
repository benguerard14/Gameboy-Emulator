#include "ppu.h"
#include "cpu.h"
#include "mmu.h"

void check_line(Memory_t *mem, PPU *ppu){
  
}

void ppu_step(Memory_t *mem, PPU *ppu, uint8_t cycles) {
  ppu->dots += cycles * 4;
  while(1){
    if(ppu->mode == 2) {
      if(ppu->dots < 80) 
        break;
      
      ppu->dots -= 80;

      ppu->mode = 3;

      uint8_t stat = mem_read(mem, LCD_STATUS);
      stat &= ~(0x3);
      mem_write(mem, LCD_STATUS, stat |= 0x3);
    }

    if(ppu->mode == 3){
      if(ppu->dots < 172 + ppu->mode3_extra)
        break;

      ppu->dots -= (172 + ppu->mode3_extra);

      //render_scanline

      ppu->mode = 0;

      uint8_t stat = mem_read(mem, LCD_STATUS);
      stat &= ~(0x3);
      mem_write(mem, LCD_STATUS, stat);

      if(stat & (1 << 3))
        request_interrupt(mem, STAT_INTERRUPT);
    }

    if(ppu->mode == 0){
      if(ppu->dots < 456 - (252 + ppu->mode3_extra))
        break;

      ppu->dots -= (456 - (252 + ppu->mode3_extra));
      ppu->mode3_extra = 0;

      ppu->lines = (ppu->lines + 1) % 154;
      if(ppu->lines == 144){
        ppu->mode = 1; 
        uint8_t stat = mem_read(mem, LCD_STATUS);
        stat &= ~(0x3);
        mem_write(mem, LCD_STATUS, stat |= 0x1);
        request_interrupt(mem, VBLANK_INTERRUPT);

        if(stat & (1 << 4))
          request_interrupt(mem, STAT_INTERRUPT);
      }
      else{
        ppu->mode = 2; 
        uint8_t stat = mem_read(mem, LCD_STATUS);
        stat &= ~(0x3);
        mem_write(mem, LCD_STATUS, stat |= 0x2);

        if(stat & (1 << 5))
          request_interrupt(mem, STAT_INTERRUPT);
      }
    }
    
    if(ppu->mode == 1){
      if(ppu->dots < 456)
        break;

      ppu->dots -= 456;
      ppu->lines = (ppu->lines + 1) % 154;

      if(ppu->lines == 0){
        ppu->mode = 2; 
        uint8_t stat = mem_read(mem, LCD_STATUS);
        stat &= ~(0x3);
        mem_write(mem, LCD_STATUS, stat |= 0x2);

        if(stat & (1 << 5))
          request_interrupt(mem, STAT_INTERRUPT);
      }
    }
  }
}
