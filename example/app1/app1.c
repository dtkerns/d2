#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <math.h>

#define M_PI 3.14159265358979323846
const double step = 2 * M_PI / 64;

uint64_t rotf(int i, uint64_t x)
{
  uint64_t z = x;
  for (int i = 0; i < 64; i++) {
    int y = (int) (sin(step * i) * i);
    for (int j = 0; j < y; j++) {
      z = (z >> 32) | (z << 8) + (j << 18);
    }
    x ^= z;
  }
  return x;
}

int main(int argc, char **argv)
{
  uint64_t x = 0xa5a5a5a5a5;
  for (int i = 0; i < 1000000; i++) {
    x = rotf(i, x);
    if ((i % 5209) == 0) {
       printf("%d %" PRIu64 "\n", i, x);
    }
  }
  printf("%" PRIu64 "\n", x);
  return 0;
}
