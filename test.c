#include <stdio.h>
#include <stdlib.h>

int foo() { return 1; }

int bar() { return 2; }

int multi(long a, long b, long c, long d, long e, long f) {
  printf("%ld, %ld, %ld, %ld, %ld, %ld\n", a, b, c, d, e, f);
  return a + b + c + d + e + f;
}

void alloc4(long **p, long a, long b, long c, long d) {
  long *ip = (long *)malloc(sizeof(long) * 4);
  ip[0] = a;
  ip[1] = b;
  ip[2] = c;
  ip[3] = d;

  *p = ip;
}
