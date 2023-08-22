CFLAGS=-std=c11 -g -static

mycc: mycc.c

test: mycc
	./test.sh

clean:
	rm -f mycc *.o *~ tmp*

fmt:
	clang-format -i -style=llvm *.c

.PHONY: test clean fmt
