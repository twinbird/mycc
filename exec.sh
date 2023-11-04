#!/bin/bash

input="$1"

cc -static -c test.c -o test.o
./mycc "$input" > tmp.s
cc -static -o tmp tmp.s test.o
./tmp
