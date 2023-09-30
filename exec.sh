#!/bin/bash

input="$1"

cc -c test.c -o test.o
./mycc "$input" > tmp.s
cc -o tmp tmp.s test.o
./tmp
