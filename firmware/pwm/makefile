# makfile configuration
CPU             = msp430x2012

CFLAGS          = -Wall -g -O3

#switch the compiler (for the internal make rules)
CC=msp430-gcc
LD=msp430-ld

pwm: main.c hardware.h i2c.h i2c.c
	${CC} -mmcu=${CPU} -o $@ main.c i2c.c ${CFLAGS}

.PHONY: clean

clean::
	-rm -f pwm
