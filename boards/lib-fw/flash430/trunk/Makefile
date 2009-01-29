ifeq ($(origin CC),default)
CC = msp430-gcc
endif

CFLAGS ?= -g -mmcu=${ARCH} -Wall -O3
CFLAGS += -I./types

override CFLAGS += -DFW_VER=0

C_FILES := flash.c i2c-flash.c fw_ver.c
H_FILES := flash.h i2c-flash.h fw_ver.h

libflash430.a: flash.o i2c-flash.o fw_ver.o
	msp430-ar r $@ flash.o i2c-flash.o fw_ver.o

%.o: %.c %.h
ifndef ARCH
	@echo "ERROR:  ARCH is not set.  This should be set on the command line."
	@echo -e "\tLike so: make ARCH=\"msp430x2234\""
	@false
endif
	${CC} -c -o $@ ${CFLAGS} $<

.PHONY: clean

clean:
	-rm -f *.o libflash430.a
