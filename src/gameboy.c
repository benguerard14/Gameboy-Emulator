#include "emulator.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define PROGRAM_SIZE 1048576

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

char *get_string_file(char *file) {
  static char buff[PROGRAM_SIZE];
  FILE *f = fopen(file, "rb");
  if (!f) {
    printf("ERROR: Failed to Open ROM\n");
    exit(-1);
  }
  size_t n = fread(buff, 1, PROGRAM_SIZE, f);
  fclose(f);
  if (n == 0) {
    printf("ERROR: Nothing in file!\n");
    exit(-1);
  }
  return buff;
}

void emulator_init(){

}
