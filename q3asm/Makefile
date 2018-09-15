# yeah, couldn't do more simple really

CC=gcc
CFLAGS=-g -Wall -Wextra -O2

default:	q3asm

q3asm:	q3asm.c cmdlib.c
	$(CC) $(CFLAGS) -o $@ $^
	@echo 'Executable created: '$@

clean:
	rm -f q3asm *~ *.o

