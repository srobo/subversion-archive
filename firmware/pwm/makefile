CPU = msp430x2012
CFLAGS := -Wall -g -O3

CC=msp430-gcc
LD=msp430-ld

include .config

ifeq ($(strip $(SWEEP_SERVOS)),y)
CFLAGS += -DSWEEP_SERVOS=1
else
CFLAGS += -DSWEEP_SERVOS=0
endif

ifeq ($(strip $(USE_CHECKSUMS)),y)
CFLAGS += -DUSE_CHECKSUMS=1
else
CFLAGS += -DUSE_CHECKSUMS=0
endif

C_FILES := main.c i2c.c sweep.c servo.c smbus_pec.c
H_FILES :=  hardware.h i2c.h sweep.h servo.h smbus_pec.h

pwm: .config $(C_FILES) $(H_FILES)
	${CC} -mmcu=${CPU} -o $@ ${C_FILES} ${CFLAGS}

.PHONY: clean

clean::
	-rm -f pwm
