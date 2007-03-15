# makfile configuration
NAME            = leds
OBJECTS         = main.o
CPU             = msp430x2013

CFLAGS          = -mmcu=${CPU} -O2 -Wall -g

#switch the compiler (for the internal make rules)
CC              = msp430-gcc

leds: main.c hardware.h i2c.h i2c.c
	${CC} -mmcu=${CPU} -o $@ main.c i2c.c ${CFLAGS} ${LDFLAGS}
