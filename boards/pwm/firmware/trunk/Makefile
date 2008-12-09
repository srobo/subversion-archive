CC := msp430-gcc
CFLAGS := -g -mmcu=msp430x2234  -Wall -O3
LDFLAGS :=

H_FILES = servo.h sweep.h hardware.h common.h i2c.h smbus_pec.h timer-b.h
C_FILES = main.c servo.c sweep.c i2c.c smbus_pec.c timer-b.c

pwm:  ${H_FILES} ${C_FILES}
	${CC} -o $@ ${C_FILES} ${CFLAGS} ${LDFLAGS}


.PHONY: clean

clean:
	-rm -f pwm 
