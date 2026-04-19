#include "emulator.h"
#include <stdio.h>
#include <stdlib.h>

#define PROGRAM_SIZE 1048576

char *get_string_file(char *file) {
  static char buff[PROGRAM_SIZE];
  FILE *f = fopen(file, "rb"); // "rb" not "r"
  if (!f) {
    printf("Error opening ROM\n");
    exit(-1);
  }
  size_t n = fread(buff, 1, PROGRAM_SIZE, f);
  fclose(f);
  if (n == 0) {
    printf("Nothing in file!\n");
    exit(-1);
  }
  return buff;
}
