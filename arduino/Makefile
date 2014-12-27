CFLAGS = -std=c99 -ggdb -Og
GCC = gcc -fsanitize=address,leak,undefined
ARGS = < square.dov

a.out: drawall.c drawall.h stub.o
	$(GCC) $(CFLAGS) -lm drawall.c stub.o

avr: Makefile drawall.c drawall.h scratch.o
	avr-gcc $(CFLAGS) -Os drawall.c scratch.o -o avr

stub.o: stub.c drawall.h
	$(GCC) $(CFLAGS) -c stub.c

run: a.out
	./a.out < ../SD_files/drawing | less

debug: a.out
	gdb -q -ex 'br main' -ex 'r $(ARGS)' ./$<

scratch.o: scratch.c
	avr-gcc $(CFLAGS) -Os -c -D__AVR_ATmega328__ -I/usr/avr/include scratch.c
