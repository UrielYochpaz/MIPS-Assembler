src = $(wildcard *.c)
obj = $(src:.c=.o)

LDFLAGS = -Wall -ansi -pedantic

assembler: $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)
