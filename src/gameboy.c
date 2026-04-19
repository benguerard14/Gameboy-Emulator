#include "gameboy.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Gameboy gb;

uint8_t* pixels_from_tile(uint8_t tile[16]){
  uint8_t* pixels = malloc(64 * sizeof(uint8_t));
  
  for(int i = 0; i < 8; i++) {
    for(int j = 0; j < 8; j++){
      uint8_t bit1 = (tile[i*2] >> j) & 0x01;
      uint8_t bit2 = ((tile[(i*2) + 1] >> j) &0x01) << 1;
      pixels[(7-j) + 8*i] = bit1+bit2;
    }
  }

  return pixels;
}

char *get_string_file(char *file, size_t *out_size) {
  FILE *f = fopen(file, "rb");
  if (!f) {
    printf("ERROR: Failed to Open ROM\n");
    exit(-1);
  }

  fseek(f, 0, SEEK_END);
  size_t file_size = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *buff = malloc(file_size);
  if (!buff) {
    printf("ERROR: Failed to allocate memory\n");
    fclose(f);
    exit(-1);
  }

  size_t n = fread(buff, 1, file_size, f);
  fclose(f);

  if (n != file_size) {
    printf("ERROR: Failed to read complete file\n");
    free(buff);
    exit(-1);
  }

  if (out_size) {
    *out_size = file_size;
  }

  return buff;
}

void emulator_init(){
}
