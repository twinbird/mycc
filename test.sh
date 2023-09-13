#!/bin/bash

assert() {
  expected="$1"
  input="$2"

  cc -c test.c -o test.o
  ./mycc "$input" > tmp.s
  cc -o tmp tmp.s test.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 'main() { return 0;}'
assert 42 'main() { return 42;}'
assert 21 'main() { return 5+20-4;}'
assert 41 ' main() { return 12 + 34 - 5 ;}'
assert 47 'main() { return 5+6*7;}'
assert 15 'main() { return 5*(9-6); }'
assert 4 'main(){return(3+5)/2;}'
assert 10 'main(){return -10+20;}'
assert 2 'main() { return +2;}'
# EQ
assert 1 'main() { return 1 == 1;}'
assert 0 'main() { return 1 == 2;}'
assert 1 'main() { return (1+1) == 2;}'
# NE
assert 1 'main() { return 1 != 2;}'
assert 0 'main() { return 1 != 1;}'
# LT
assert 1 'main() { return -1 < 0;}'
assert 0 'main() { return 0 < 0;}'
assert 0 'main() { return 1 < 0;}'
# LE
assert 1 'main() { return 0 <= 0;}'
assert 1 'main() { return -1 <= 0;}'
assert 0 'main() { return 1 <= 0;}'
# GT
assert 1 'main() { return 0 > -1;}'
assert 0 'main() { return 0 > 0;}'
assert 0 'main() { return 0 > 1;}'
# GE
assert 1 'main() { return 0 >= 0;}'
assert 1 'main() { return 1 >= 0;}'
assert 0 'main() { return 0 >= 1;}'

# assign
assert 2 'main() { a = 1+1;return a;}'
assert 12 'main() { b = (1+1) * 6; return b;}'
assert 2 'main() { a = 1+1;b = (1+1) * 6; return a;}'
assert 3 'main() { foo = 1; bar = 2; return foo + bar;}'

# return
assert 2 'main() { a = 1; return 2; return a;}'
assert 42 'main() { a = 10; b = 32; return a + b; return b;}'
assert 4 'main() { a = 1; b = 2; return 4;}'
assert 2 'main() { returnx = 1; x = 2; return x;}'

# if
assert 2 'main() { a = 1; if (a > 0) return 2; return 1;}'
assert 1 'main() { a = 0; if (a > 0) return 2; return 1;}'
assert 3 'main() { a = 3; if (a > 0) if (2 > 1) return a; return 1;}'
assert 1 'main() { a = 1; if (a > 0) if (1 > 2) return 2; return a;}'
assert 2 'main() { if (0 > 1) return 1; if (1 > 0) return 2;}'

# if - else
assert 2 'main() { a = 1; b = 2; if (a > b) return a; else return b;}'
assert 3 'main() { a = 3; b = 2; if (a > b) return a; else return b;}'
assert 1 'main() { a=0; if (3>2) a=1; else if (2>3) a=2; else a=99; return a;}'
assert 2 'main() { a=0; if (1>2) a=1; else if (4>3) a=2; else a=99; return a;}'
assert 99 'main() { a=0; if (1>2) a=1; else if (2>3) a=2; else a=99; return a;}'

# while
assert 3 'main() { i = 0; while(i < 3) i = i + 1; return i;}'
assert 2 'main() { i = 2; while(i < 2) i = i + 1; return i;}'

# for
assert 3 'main() { sum = 0; for (i = 0; i < 3; i = i + 1) sum = sum + i; return sum;}'
assert 1 'main() { sum = 0; for (i=0;;) return 1;}'
assert 1 'main() { i=0;for (;i!=1;) i=1; return i;}'
assert 5 'main() { sum=0; for(;;sum = sum + 1) if (sum > 4) return sum;}'

# block
assert 3 'main() { { i = 0; i = 3; return i;}}'
assert 2 'main() { i = 0; if (1 < 0) { i = 1; return i; } else if (1 > 0) { i = 2;return i; }}'

# function call
assert 1 'main() { return foo();}'
assert 2 'main() { return bar();}'
assert 21 'main() { return multi(1, 2, 3, 4, 5, 6); }'

# function definition
assert 2 'ret2() { return 2; } main() { return ret2();}'
assert 4 'ret2() { return 2; } main() { return ret2() + ret2(); }'
#assert 6 'ret3() { one = 1; two = 2; return one + two; } main() { return ret3() + 3; }'

echo OK
