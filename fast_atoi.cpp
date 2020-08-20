#include "fast_atoi.h"

const unsigned int fast_atoi_map [] = {
  0, 10, 20, 30, 40, 50, 60, 70, 80, 90,
  0, 100, 200, 300, 400, 500, 600, 700, 800, 900,
  0, 1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000, 9000,
  0, 10000, 20000, 30000, 40000, 50000, 60000
};

unsigned int fast_atoi(char *str, unsigned char length) {
  unsigned int val = 0;
  const unsigned int *imap = fast_atoi_map + (length - 2) * 10;
  while (length-- > 1) {
    val = val + *(imap + (*str++ - '0'));
    imap -= 10;
  }
  return val + (*str - '0');
}
