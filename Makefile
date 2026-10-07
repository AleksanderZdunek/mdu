TARGET = mdu
OBJ = mdu.o

CC = gcc
CFLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations \
	-Wmissing-prototypes -Wold-style-definition
LDFLAGS =

all: $(TARGET)

-include *.d
%.o: %.c Makefile
	$(CC) -MMD $(CFLAGS) -c -o $@ $<

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET) $(OBJ:.o=.d)
