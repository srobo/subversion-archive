CC := msp430-gcc
CFLAGS := -g -mmcu=msp430x2234  -Wall -O3
LDFLAGS :=

H_FILES = common.h hardware.h i2c.h init.h servo.h smbus_pec.h sweep.h timer-b.h
C_FILES = main.c i2c.c init.c servo.c sweep.c smbus_pec.c timer-b.c

motor:  ${H_FILES} ${C_FILES}
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS}


.PHONY: clean

clean:
	-rm -f pwm
