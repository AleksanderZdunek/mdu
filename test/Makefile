CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations \
	-Wmissing-prototypes -Wold-style-definition
LDFLAGS =

test_string_stack: test_string_stack.o string_stack.o
	$(CC) $(LDFLAGS) -o $@ $^

test_string_stack.o: test_string_stack.c ../string_stack.h Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

string_stack.o: ../string_stack.c ../string_stack.h Makefile
	$(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	rm -f test_string_stack *.o
