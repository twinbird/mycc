#include "stdio.h"

int foo() {
  return 1;
}

int bar() {
  return 2;
}

int multi(int a, int b, int c, int d, int e, int f) {
  printf("%d, %d, %d, %d, %d, %d\n", a, b, c, d, e, f);
  return a + b + c + d + e + f;
}
