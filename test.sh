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

assert 0 '0;'
assert 42 '42;'
assert 21 '5+20-4;'
assert 41 ' 12 + 34 - 5 ;'
assert 47 '5+6*7;'
assert 15 '5*(9-6);'
assert 4 '(3+5)/2;'
assert 10 '-10+20;'
assert 2 '+2;'
# EQ
assert 1 '1 == 1;'
assert 0 '1 == 2;'
assert 1 '(1+1) == 2;'
# NE
assert 1 '1 != 2;'
assert 0 '1 != 1;'
# LT
assert 1 '-1 < 0;'
assert 0 '0 < 0;'
assert 0 '1 < 0;'
# LE
assert 1 '0 <= 0;'
assert 1 '-1 <= 0;'
assert 0 '1 <= 0;'
# GT
assert 1 '0 > -1;'
assert 0 '0 > 0;'
assert 0 '0 > 1;'
# GE
assert 1 '0 >= 0;'
assert 1 '1 >= 0;'
assert 0 '0 >= 1;'

# assign
assert 2 'a = 1+1;a;'
assert 12 'b = (1+1) * 6;b;'
assert 2 'a = 1+1;b = (1+1) * 6;a;'
assert 3 'foo = 1; bar = 2; foo + bar;'

# return
assert 2 'a = 1; return 2; a;'
assert 42 'a = 10; b = 32; return a + b; b;'
assert 4 'a = 1; b = 2; 4;'
assert 2 'returnx = 1; x = 2; return x;'

# if
assert 2 'a = 1; if (a > 0) return 2; return 1;'
assert 1 'a = 0; if (a > 0) return 2; return 1;'
assert 3 'a = 3; if (a > 0) if (2 > 1) return a; return 1;'
assert 1 'a = 1; if (a > 0) if (1 > 2) return 2; return a;'
assert 2 'if (0 > 1) return 1; if (1 > 0) return 2;'

# if - else
assert 2 'a = 1; b = 2; if (a > b) return a; else return b;'
assert 3 'a = 3; b = 2; if (a > b) return a; else return b;'
assert 1 'a=0; if (3>2) a=1; else if (2>3) a=2; else a=99; return a;'
assert 2 'a=0; if (1>2) a=1; else if (4>3) a=2; else a=99; return a;'
assert 99 'a=0; if (1>2) a=1; else if (2>3) a=2; else a=99; return a;'

# while
assert 3 'i = 0; while(i < 3) i = i + 1; return i;'
assert 2 'i = 2; while(i < 2) i = i + 1; return i;'

# for
assert 3 'sum = 0; for (i = 0; i < 3; i = i + 1) sum = sum + i;'
assert 1 'sum = 0; for (i=0;;) return 1;'
assert 1 'i=0;for (;i!=1;) i=1; return i;'
assert 5 'sum=0; for(;;sum = sum + 1) if (sum > 4) return sum;'

# block
assert 3 '{ i = 0; i = 3; return i;}'
assert 2 'i = 0; if (1 < 0) { i = 1; return i; } else if (1 > 0) { i = 2;return i; }'

# func call
assert 1 'return foo();'
assert 2 'return bar();'

echo OK
