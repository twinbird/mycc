#include <stdio.h>
#include <stdlib.h>

int foo() { return 1; }

int bar() { return 2; }

int multi(int a, int b, int c, int d, int e, int f) {
  printf("%d, %d, %d, %d, %d, %d\n", a, b, c, d, e, f);
  return a + b + c + d + e + f;
}

void alloc4(int **p, int a, int b, int c, int d) {
  int *ip = (int *)malloc(sizeof(int) * 4);
  ip[0] = a;
  ip[1] = b;
  ip[2] = c;
  ip[3] = d;

  *p = ip;
}
