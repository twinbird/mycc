CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

mycc: $(OBJS)
	$(CC) -o mycc $(OBJS) $(LDFLAGS)

$(OBJS): mycc.h

test: mycc
	./test.sh

clean:
	rm -f mycc *.o *~ tmp*

fmt:
	clang-format -i -style=llvm *.c

.PHONY: test clean fmt
