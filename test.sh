#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  cc -static -c test.c -o test.o
  echo "$input" > tmp.csrc
  ./mycc tmp.csrc > tmp.s
  cc -static -o tmp tmp.s test.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 'int main() { return 0;}'
assert 42 'int main() { return 42;}'
assert 21 'int main() { return 5+20-4;}'
assert 41 ' int main() { return 12 + 34 - 5 ;}'
assert 47 'int main() { return 5+6*7;}'
assert 15 'int main() { return 5*(9-6); }'
assert 4 'int main(){return(3+5)/2;}'
assert 10 'int main(){return -10+20;}'
assert 2 'int main() { return +2;}'
# EQ
assert 1 'int main() { return 1 == 1;}'
assert 0 'int main() { return 1 == 2;}'
assert 1 'int main() { return (1+1) == 2;}'
# NE
assert 1 'int main() { return 1 != 2;}'
assert 0 'int main() { return 1 != 1;}'
# LT
assert 1 'int main() { return -1 < 0;}'
assert 0 'int main() { return 0 < 0;}'
assert 0 'int main() { return 1 < 0;}'
# LE
assert 1 'int main() { return 0 <= 0;}'
assert 1 'int main() { return -1 <= 0;}'
assert 0 'int main() { return 1 <= 0;}'
# GT
assert 1 'int main() { return 0 > -1;}'
assert 0 'int main() { return 0 > 0;}'
assert 0 'int main() { return 0 > 1;}'
# GE
assert 1 'int main() { return 0 >= 0;}'
assert 1 'int main() { return 1 >= 0;}'
assert 0 'int main() { return 0 >= 1;}'

# assign
assert 2 'int main() { int a; a = 1+1;return a;}'
assert 12 'int main() { int b; b = (1+1) * 6; return b;}'
assert 2 'int main() { int a; a = 1+1;int b; b = (1+1) * 6; return a;}'
assert 3 'int main() { int foo; foo = 1; int bar; bar = 2; return foo + bar;}'

# return
assert 2 'int main() { int a; a = 1; return 2; return a;}'
assert 42 'int main() { int a; a = 10; int b; b = 32; return a + b; return b;}'
assert 4 'int main() { int a; a = 1; int b; b = 2; return 4;}'
assert 2 'int main() { int returnx; returnx = 1; int x; x = 2; return x;}'

# if
assert 2 'int main() { int a; a = 1; if (a > 0) return 2; return 1;}'
assert 1 'int main() { int a; a = 0; if (a > 0) return 2; return 1;}'
assert 3 'int main() { int a; a = 3; if (a > 0) if (2 > 1) return a; return 1;}'
assert 1 'int main() { int a; a = 1; if (a > 0) if (1 > 2) return 2; return a;}'
assert 2 'int main() { if (0 > 1) return 1; if (1 > 0) return 2;}'

# if - else
assert 2 'int main() { int a; int b; a = 1; b = 2; if (a > b) return a; else return b;}'
assert 3 'int main() { int a; int b; a = 3; b = 2; if (a > b) return a; else return b;}'
assert 1 'int main() { int a; a=0; if (3>2) a=1; else if (2>3) a=2; else a=99; return a;}'
assert 2 'int main() { int a; a=0; if (1>2) a=1; else if (4>3) a=2; else a=99; return a;}'
assert 99 'int main() { int a; a=0; if (1>2) a=1; else if (2>3) a=2; else a=99; return a;}'

# while
assert 3 'int main() { int i; i = 0; while(i < 3) i = i + 1; return i;}'
assert 2 'int main() { int i; i = 2; while(i < 2) i = i + 1; return i;}'

# for
assert 3 'int main() { int sum; int i; sum = 0; for (i = 0; i < 3; i = i + 1) sum = sum + i; return sum;}'
assert 1 'int main() { int sum; int i; sum = 0; for (i=0;;) return 1;}'
assert 1 'int main() { int i; i=0;for (;i!=1;) i=1; return i;}'
assert 5 'int main() { int sum; sum=0; for(;;sum = sum + 1) if (sum > 4) return sum;}'

# block
assert 3 'int main() { { int i; i = 0; i = 3; return i;}}'
assert 2 'int main() { int i; i = 0; if (1 < 0) { i = 1; return i; } else if (1 > 0) { i = 2;return i; }}'

# function call
assert 1 'int main() { return foo();}'
assert 2 'int main() { return bar();}'
assert 21 'int main() { return multi(1, 2, 3, 4, 5, 6); }'
assert 21 'int main() { int i; return multi(1, 2, 3, 4, 5, 6); }'

# function definition
assert 2 'int ret2() { return 2; } int main() { return ret2();}'
assert 4 'int ret2() { return 2; } int main() { return ret2() + ret2(); }'
assert 6 'int ret3() { int one; int two; one = 1; two = 2; return one + two; } int main() { return ret3() + 3; }'
assert 1 'int ret(int a) { return a; } int main() { return ret(1);}'
assert 28 'int add(int a, int b, int c, int d, int e, int f) { int g; g = 7; return a + b + c + d + e + f + g;} int main() { return add(1,2,3,4,5,6); }'
assert 120 'int factorial(int n) { if (n == 0) { return 1; } return n * factorial(n-1); } int main() { return factorial(5); }'

# reference & dereference
assert 20 'int main() { int a; int *ref; a = 20; ref = &a; return *ref;}'
assert 4 'int main() { int x; int *y; x = 1; y = &x; *y = 4; return x; }'

# sizeof
assert 4 'int main() { return sizeof(1);}'
assert 4 'int main() { int x; x = 10; return sizeof(x + 3); }'
assert 8 'int main() { int *p; return sizeof(p); }'
assert 4 'int main() { int *p; return sizeof(*p); }'
assert 1 'int main() { char c; return sizeof(c); }'
assert 8 'int main() { long l; return sizeof(l); }'

# pointer calculation
assert 3 'int main() { int *p; alloc4(&p, 1,2,3,4); int *q; q = p + 2; return *q; }'
assert 2 'int main() { int *p; alloc4(&p, 1,2,3,4); int *q; q = p + 2; q = q - 1; return *q; }'

# array
assert 80 'int main() { int a[10]; return 80;}'
assert 3 'int main() { int a[2]; *a = 1; *(a+1)=2; int *p; p=a; return *p+*(p+1); }'
assert 30 'int main() { int a[5]; a[0]=0; a[1]=10; a[2]=20; a[3]=30; a[4]=40; return a[3];}'
assert 40 'int main() { int a[5]; a[0]=0; a[1]=10; a[2]=20; a[3]=30; a[4]=40; return 4[a];}'

# global variable
assert 7 'int global_int; int main() { int a; a = 10; global_int = a - 3; return global_int; }'
assert 4 'int *global_ptr; int main() { int a; a = 10; global_ptr = &a; *global_ptr = *global_ptr - 6; return *global_ptr; }'
assert 2 'int global_ary[3]; int main() { global_ary[0] = 1; global_ary[1] = 2; global_ary[2] = 3; return global_ary[1]; }'

# char type
assert 2 'int main() { char c; c = 1; return c + 1; }'
assert 3 'int main() { char x[3]; x[0] = -1; x[1] = 2; int y; y = 4; return x[0] + y; }'
assert 5 'char add(char n, char m) { return n + m; } int main() {return add(2, 3);}'

# long type
assert 2 'int main() { long c; c = 1; return c + 1; }'
assert 5 'long add(long n, long m) { return n + m; } int main() {return add(2, 3);}'

# string literal
assert 12 'int main() { char *str; str = "hello, world"; return mystrlen(str); }'

# comment
assert 1 'int main() { /* this is a comment */ return 1; }'

echo OK
