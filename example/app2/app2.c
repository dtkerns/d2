#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <math.h>


uint64_t rotf(int i, uint64_t x, float *f)
{
  uint64_t z = x;
  for (int i = 0; i < 128; i++) {
    int y = i * 11;
    for (int j = 0; j < y; j++) {
      z = (z >> 18) | (z << 16) + (j << 19);
    }
    x ^= z;
    *f = *f * 5.0;
  }
  return x;
}

uint64_t rotf2(int i, uint64_t x, float *f)
{
  uint64_t z = x;
  for (int i = 0; i < 128; i++) {
    int y = i * 11;
    for (int j = 0; j < y; j++) {
      z = (z >> 17) | (z << 19) + (j << 21);
    }
    x ^= z;
    *f = *f * 0.02;
  }
  return x;
}

int main(int argc, char **argv)
{
  uint64_t x = 0x5a5a5a5a5a;
  float f = 0.0;
  for (int i = 0; i < 1000000; i++) {
    x = ((i & 1) == 1) ? rotf(i, x, &f) : rotf2(i, x, &f);
    if ((i % 5209) == 0) {
       printf("%d %" PRIu64 "\n", i, x);
    }
  }
  printf("%" PRIu64 "\n", x);
  return 0;
}
