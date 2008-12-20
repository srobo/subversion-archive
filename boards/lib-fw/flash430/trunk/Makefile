ifeq ($(origin CC),default)
CC = msp430-gcc
endif

CFLAGS ?= -g -mmcu=${ARCH} -Wall -O3
CFLAGS += -I./types

C_FILES := flash.c i2c-flash.c
H_FILES := flash.h i2c-flash.h

libflash430.a: flash.o i2c-flash.o
	msp430-ar r $@ flash.o i2c-flash.o

%.o: %.c %.h
	${CC} -c -o $@ ${CFLAGS} $<

.PHONY: clean

clean:
	-rm -f *.o libflash430.a
